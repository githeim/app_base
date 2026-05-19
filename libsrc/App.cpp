#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "App.h"
#include "CBase.h"
#include "Log_Util.h"
#include "SDL2_Ctx.h"
#include "SceneDef.h"

#include "scenes/SceneUtil.h"
#include "scenes/SceneMap.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer.h"

namespace {

/// @brief 메인 루프의 목표 루프/초 (frames per second).
constexpr double APP_LPS = 60.0;

/**
 * @brief RGB 색상 값을 담는 구조체이다.
 */
struct ColorRgb {
  Uint8 r; ///< Red 컴포넌트
  Uint8 g; ///< Green 컴포넌트
  Uint8 b; ///< Blue 컴포넌트
};

/// @brief 일반 씬 배경색 (중간 회색).
constexpr ColorRgb kSceneBackground{110, 110, 110};
/// @brief 씬 전이 중 배경색 (어두운 회색)
constexpr ColorRgb kTransitionBackground{60, 60, 60};

}  // namespace


class App::Impl {
public:
  /**
   * @brief ECS 컨텍스트, 디스패처, 씬 정의를 초기화하고 이벤트를 연결한다.
   */
  Impl()
      : m_pECS(std::make_shared<entt::registry>()) {
    m_pECS->ctx().emplace<AppCtx>();
    m_pECS->ctx().emplace<entt::dispatcher>();
    m_pECS->ctx().emplace<SceneRuntime>();
    m_pECS->ctx().emplace<GameState>();   // 앱 전역 누적 상태 (씬 간 공유)
    m_pECS->ctx().emplace<SceneLoadingContext>(); // 로딩 진행도 공유 컨텍스트

    SetupSceneMap();
    SetupProcedureMap();

    auto &dispatcher = m_pECS->ctx().get<entt::dispatcher>();
    dispatcher.sink<SceneTransitionRequest>().connect<&App::Impl::OnSceneTransitionRequest>(*this);
    dispatcher.sink<AppQuitRequest>().connect<&App::Impl::OnAppQuitRequest>(*this);
  }

  /**
   * @brief 등록했던 dispatcher sink 를 해제한다.
   */
  ~Impl() {
    if (m_pECS != nullptr && m_pECS->ctx().contains<entt::dispatcher>()) {
      auto &dispatcher = m_pECS->ctx().get<entt::dispatcher>();
      dispatcher.sink<SceneTransitionRequest>().disconnect<&App::Impl::OnSceneTransitionRequest>(*this);
      dispatcher.sink<AppQuitRequest>().disconnect<&App::Impl::OnAppQuitRequest>(*this);
    }
  }

  /**
   * @brief 메인 루프 시작 전에 SDL 과 ImGui 를 초기화한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @return 성공 시 0, 실패 시 1
   */
  int PreLoop(entt::registry &ECS) {
    auto &appCtx = ECS.ctx().get<AppCtx>();
    if (Init_SDL_ctx(appCtx) != 0) {
      LER("SDL context initialization failed");
      ECS.ctx().get<SceneRuntime>().shouldQuit = true;
      Base.Set_bLoopTrigger(false);
      return 1;
    }

    if (!InitImGui(appCtx)) {
      LER("ImGui initialization failed");
      ECS.ctx().get<SceneRuntime>().shouldQuit = true;
      Base.Set_bLoopTrigger(false);
      return 1;
    }

    // SceneID::Title이 최초 진입점이다
    InvokeOnEnter(ECS, SceneId::Title);
    return 0;
  }

  /**
   * @brief 한 프레임의 입력 처리, 전이 갱신, 렌더링을 수행한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @return 종료 요청이 있으면 1, 계속 실행이면 0
   */
  int Loop(entt::registry &ECS) {
    auto &appCtx = ECS.ctx().get<AppCtx>();
    auto &runtime = ECS.ctx().get<SceneRuntime>();

    if (appCtx.pWindow == nullptr || appCtx.pRenderer == nullptr) {
      runtime.shouldQuit = true;
      return 1;
    }

    ProcessSdlEvents(ECS);

    auto &dispatcher = ECS.ctx().get<entt::dispatcher>();
    dispatcher.update<SceneTransitionRequest>();
    dispatcher.update<AppQuitRequest>();

    UpdateTransition(ECS);
    UpdateActiveScene(ECS);
    Render(ECS);

    double actualDiff_SEC = 0.0;
    FPS_Ctrl(MAIN_APP, 1.0 / APP_LPS, actualDiff_SEC);
    return runtime.shouldQuit ? 1 : 0;
  }

  /**
   * @brief 메인 루프 종료 후 ImGui 와 SDL 리소스를 정리한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @return 항상 0
   */
  int PostLoop(entt::registry &ECS) {
    ShutdownImGui();
    auto &appCtx = ECS.ctx().get<AppCtx>();
    DeInit_SDL_ctx(appCtx);
    return 0;
  }

  /**
   * @brief 베이스 루프 스레드 종료를 대기한다.
   *
   * @return 대기 처리 결과 코드
   */
  int Wait() {
    return Base.Wait();
  }

  /**
   * @brief 현재 애플리케이션이 사용하는 루프 프로시저 맵을 반환한다.
   *
   * @return 프로시저 맵 참조
   */
  Proc_Map_T &ProcMap() {
    return m_procMap;
  }

  /**
   * @brief 씬 전이 요청 이벤트를 받아 실제 전이 절차를 시작한다.
   *
   * @param[in] request 전이 요청 정보
   */
  void OnSceneTransitionRequest(const SceneTransitionRequest &request) {
    if (request.playArgs.has_value()) {
      m_pECS->ctx().get<SceneRuntime>().playArgs = request.playArgs;
    }
    StartTransition(*m_pECS, request.target);
  }

  /**
   * @brief 종료 요청 이벤트를 받아 종료 전환 절차를 시작한다.
   */
  void OnAppQuitRequest(const AppQuitRequest &) {
    StartQuit(*m_pECS);
  }

  CBase Base;
  std::shared_ptr<entt::registry> m_pECS;

private:
  Proc_Map_T m_procMap;
  std::map<SceneId, SceneDefinition> m_sceneMap;

  /**
   * @brief Pre/Loop/Post 루프 콜백 맵을 구성한다.
   */
  void SetupProcedureMap() {
    m_procMap = {
        {PRE_LOOP_PROC,  [this](entt::registry &ECS) { return PreLoop(ECS); }},
        {LOOP_PROC,      [this](entt::registry &ECS) { return Loop(ECS); }},
        {POST_LOOP_PROC, [this](entt::registry &ECS) { return PostLoop(ECS); }},
    };
  }

  /**
   * @brief 각 씬의 OnEnter/OnExit/Render 훅을 연결한다.
   */
  void SetupSceneMap() {
    m_sceneMap = GetSceneMap();
  }

  /**
   * @brief ImGui 컨텍스트와 SDL2/SDLRenderer 백엔드를 초기화한다.
   *
   * @param[in] appCtx SDL 윈도우/렌더러를 포함한 컨텍스트
   * @return 성공 시 true, 실패 시 false
   */
  bool InitImGui(AppCtx &appCtx) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    io.Fonts->AddFontFromFileTTF(
        "resource/fonts/NanumGothicCoding-Regular.ttf",
        18.0f,
        nullptr,
        io.Fonts->GetGlyphRangesKorean());

    if (!ImGui_ImplSDL2_InitForSDLRenderer(appCtx.pWindow, appCtx.pRenderer)) {
      return false;
    }
    if (!ImGui_ImplSDLRenderer_Init(appCtx.pRenderer)) {
      ImGui_ImplSDL2_Shutdown();
      return false;
    }
    return true;
  }

  /**
   * @brief ImGui 백엔드와 컨텍스트를 안전하게 종료한다.
   */
  void ShutdownImGui() {
    if (ImGui::GetCurrentContext() != nullptr) {
      ImGui_ImplSDLRenderer_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
    }
  }

  /**
   * @brief SDL 이벤트를 처리하고 필요한 경우 종료 이벤트를 enqueue 한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   */
  void ProcessSdlEvents(entt::registry &ECS) {
    SDL_Event event;
    auto &dispatcher = ECS.ctx().get<entt::dispatcher>();

    while (SDL_PollEvent(&event) != 0) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        dispatcher.enqueue<AppQuitRequest>(AppQuitRequest{});
      }
    }
  }

  /**
   * @brief 씬 전이 타이머를 갱신하고 단계별 상태를 진행시킨다.
   *
   * @param[in,out] ECS ECS 레지스트리
   */
  void UpdateTransition(entt::registry &ECS) {
    auto &runtime = ECS.ctx().get<SceneRuntime>();
    if (runtime.phase == TransitionPhase::None) {
      return;
    }

    // 현재 phase 화면을 최소 1프레임 표시
    if (!runtime.phaseScreenPresented) {
      runtime.phaseScreenPresented = true;
      runtime.lifecycleNote = std::string("Transition screen presented for ") + ToString(runtime.phase);
      return;
    }

    if (runtime.phase == TransitionPhase::TearingDown) {
      if (!runtime.phaseWorkDone) {
        // worker thread 가 아직 시작 안 됐으면 시작
        if (!runtime.workerThread.joinable()) {
          auto &lc = ECS.ctx().get<SceneLoadingContext>();
          lc.fProgress.store(0.0f);
          lc.szStatus.store("정리 시작...");
          runtime.lifecycleNote = std::string("Running OnExit during ") + ToString(runtime.phase);
          SceneId scene = runtime.activeScene;
          runtime.workerThread = std::thread([this, &ECS, scene, &lc]() {
            InvokeOnExit(ECS, scene);
            if (lc.fProgress.load() < 1.0f) lc.fProgress.store(1.0f);
          });
        }
        // worker thread 완료 여부 폴링
        auto &lc = ECS.ctx().get<SceneLoadingContext>();
        if (lc.fProgress.load() >= 1.0f) {
          runtime.workerThread.join();
          runtime.phaseWorkDone = true;
          runtime.exitCounts[runtime.activeScene] += 1;
          if (runtime.exitAfterFinishing) {
            runtime.lifecycleNote = "Application exit teardown completed during TearingDown";
            runtime.shouldQuit = true;
          } else {
            runtime.lifecycleNote = std::string("OnExit completed during ") + ToString(runtime.phase);
            runtime.phase = TransitionPhase::Loading;
            runtime.phaseScreenPresented = false;
            runtime.phaseWorkDone = false;
            // Loading phase 스냅샷 갱신
            lc.szPhase = ToString(runtime.phase);
          }
        }
        return;
      }

      return;
    }

    if (runtime.phase == TransitionPhase::Loading) {
      if (!runtime.pendingScene.has_value()) {
        return;
      }

      if (!runtime.phaseWorkDone) {
        // worker thread 가 아직 시작 안 됐으면 시작
        if (!runtime.workerThread.joinable()) {
          auto &lc = ECS.ctx().get<SceneLoadingContext>();
          lc.fProgress.store(0.0f);
          lc.szStatus.store("로딩 시작...");
          runtime.lifecycleNote = std::string("Running OnEnter during ") + ToString(runtime.phase);
          SceneId pending = *runtime.pendingScene;
          runtime.workerThread = std::thread([this, &ECS, pending, &lc]() {
            InvokeOnEnter(ECS, pending);
            if (lc.fProgress.load() < 1.0f) lc.fProgress.store(1.0f);
          });
        }
        // worker thread 완료 여부 폴링
        auto &lc = ECS.ctx().get<SceneLoadingContext>();
        if (lc.fProgress.load() >= 1.0f) {
          runtime.workerThread.join();
          runtime.phaseWorkDone = true;
          runtime.activeScene = *runtime.pendingScene;
          runtime.enterCounts[runtime.activeScene] += 1;
          runtime.pendingScene.reset();
          runtime.phase = TransitionPhase::None;
          runtime.phaseScreenPresented = false;
          runtime.phaseWorkDone = false;
          runtime.exitAfterFinishing = false;
          runtime.lifecycleNote = std::string("Showing ") + ToString(runtime.activeScene) + " scene";
        }
        return;
      }
    }
  }

  /**
   * @brief 현재 프레임의 배경과 UI 를 렌더링한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   */
  void Render(entt::registry &ECS) {
    auto &appCtx = ECS.ctx().get<AppCtx>();
    auto &runtime = ECS.ctx().get<SceneRuntime>();

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    const ColorRgb bg = (runtime.phase == TransitionPhase::None) ? kSceneBackground : kTransitionBackground;
    SDL_SetRenderDrawColor(appCtx.pRenderer, bg.r, bg.g, bg.b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(appCtx.pRenderer);

    if (runtime.phase == TransitionPhase::None) {
      RenderActiveScene(ECS, runtime);
    } else {
      RenderTransitionScreen(ECS, runtime);
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(appCtx.pRenderer);
  }

  /**
   * @brief 현재 활성 씬의 렌더 함수를 호출한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @param[in]     runtime 현재 씬 런타임 상태
   */
  void RenderActiveScene(entt::registry &ECS, SceneRuntime &runtime) {
    auto sceneIter = m_sceneMap.find(runtime.activeScene);
    if (sceneIter != m_sceneMap.end()) {
      sceneIter->second.onRender(ECS);
    }
  }

  /**
   * @brief 전이 중이 아닐 때 활성 씬의 onUpdate 훅을 호출한다.
   *
   * onUpdate 가 nullptr 이면 조용히 스킵한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   */
  void UpdateActiveScene(entt::registry &ECS) {
    const auto &runtime = ECS.ctx().get<SceneRuntime>();
    if (runtime.phase != TransitionPhase::None) return;

    auto sceneIter = m_sceneMap.find(runtime.activeScene);
    if (sceneIter != m_sceneMap.end() && sceneIter->second.onUpdate) {
      sceneIter->second.onUpdate(ECS);
    }
  }

  /**
   * @brief 씬 전이 중 상태와 타겟 정보를 표시하는 화면을 렌더링한다.
   *
   * @param[in] runtime 현재 씬 전이 상태
   */
  void RenderTransitionScreen(entt::registry &ECS, const SceneRuntime &runtime) {
    auto &lc = ECS.ctx().get<SceneLoadingContext>();
    BeginFullscreenUi("TransitionScreen");
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextUnformatted(lc.szPhase);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();
    ImGui::Text("Current Scene: %s", lc.szCurrentScene);
    ImGui::Text("Target: %s",        lc.szTargetScene);
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Transition order: TearingDown 화면 표시 -> current scene OnExit 수행 -> Loading 화면 표시 -> target scene OnEnter 수행 -> target scene 화면");
    ImGui::Spacing();
    // 진행도 표시
    float progress      = lc.fProgress.load();
    const char *status  = lc.szStatus.load();
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
    ImGui::Spacing();
    if (status && status[0] != '\0') {
      ImGui::TextUnformatted(status);
    }
    ImGui::Spacing();
    ImGui::Text("Lifecycle: %s", runtime.lifecycleNote.c_str());
    EndFullscreenUi();
  }

  /**
   * @brief 현재 씬에서 대상 씬으로의 전이 절차를 시작한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @param[in]     target 전이 대상 씬
   */
  void StartTransition(entt::registry &ECS, SceneId target) {
    auto &runtime = ECS.ctx().get<SceneRuntime>();
    if (runtime.shouldQuit || runtime.phase != TransitionPhase::None) {
      return;
    }
    if (runtime.activeScene == target) {
      return;
    }

    runtime.prevScene = runtime.activeScene;
    runtime.pendingScene = target;
    runtime.phase = TransitionPhase::TearingDown;
    runtime.phaseScreenPresented = false;
    runtime.phaseWorkDone = false;
    runtime.exitAfterFinishing = false;
    runtime.lifecycleNote = std::string("Starting ") + ToString(runtime.phase) + " for " + ToString(runtime.activeScene);

    // 전이 화면 스냅샷 설정 (main thread 전용)
    auto &lc = ECS.ctx().get<SceneLoadingContext>();
    lc.szPhase        = ToString(runtime.phase);
    lc.szCurrentScene = ToString(runtime.activeScene);
    lc.szTargetScene  = ToString(target);
  }

  /**
   * @brief 종료 요청에 대한 TearingDown 절차를 시작한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   */
  void StartQuit(entt::registry &ECS) {
    auto &runtime = ECS.ctx().get<SceneRuntime>();
    if (runtime.shouldQuit || runtime.phase != TransitionPhase::None) {
      return;
    }

    runtime.pendingScene.reset();
    runtime.phase = TransitionPhase::TearingDown;
    runtime.phaseScreenPresented = false;
    runtime.phaseWorkDone = false;
    runtime.exitAfterFinishing = true;
    runtime.lifecycleNote = std::string("Starting ") + ToString(runtime.phase) + " for application exit";

    // 전이 화면 스냅샷 설정 (main thread 전용)
    auto &lc = ECS.ctx().get<SceneLoadingContext>();
    lc.szPhase        = ToString(runtime.phase);
    lc.szCurrentScene = ToString(runtime.activeScene);
    lc.szTargetScene  = "Application Exit";
  }

  /**
   * @brief 지정한 씬의 OnEnter 훅을 호출한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @param[in]     scene 진입할 씬
   */
  void InvokeOnEnter(entt::registry &ECS, SceneId scene) {
    auto sceneIter = m_sceneMap.find(scene);
    if (sceneIter != m_sceneMap.end() && sceneIter->second.onEnter) {
      sceneIter->second.onEnter(ECS);
    }
  }

  /**
   * @brief 지정한 씬의 OnExit 훅을 호출한다.
   *
   * @param[in,out] ECS ECS 레지스트리
   * @param[in]     scene 이탈할 씬
   */
  void InvokeOnExit(entt::registry &ECS, SceneId scene) {
    auto sceneIter = m_sceneMap.find(scene);
    if (sceneIter != m_sceneMap.end() && sceneIter->second.onExit) {
      sceneIter->second.onExit(ECS);
    }
  }

};  // class App::Impl

/**
 * @brief 내부 구현 객체를 생성한다.
 */
App::App()
    : m_pImpl(std::make_unique<Impl>()) {
}

/**
 * @brief App 구현 객체를 기본 소멸 규칙으로 정리한다.
 */
App::~App() = default;

/**
 * @brief FPS 제어기를 초기화하고 메인 루프를 시작한다.
 *
 * @return 시작 결과 코드
 */
int App::Start() {
  Init_FPS_Ctrl();
  return m_pImpl->Base.Start(*m_pImpl->m_pECS, m_pImpl->ProcMap());
}

/**
 * @brief 메인 루프 중지를 요청한다.
 *
 * @return 중지 결과 코드
 */
int App::Stop() {
  return m_pImpl->Base.Stop();
}

/**
 * @brief 메인 루프 종료까지 대기한다.
 *
 * @return 대기 결과 코드
 */
int App::Wait() {
  return m_pImpl->Wait();
}
