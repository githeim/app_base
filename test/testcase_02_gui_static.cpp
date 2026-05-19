#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

/**
 * @brief App.cpp 원본 소스를 가능한 경로들에서 찾아 문자열로 로드한다.
 *
 * 정적 요구사항 검증 테스트에서 소스 코드 문자열 검색에 사용된다.
 *
 * @return App.cpp 전체 내용, 찾지 못하면 빈 문자열
 */
std::string LoadAppCpp() {
  namespace fs = std::filesystem;

  const fs::path thisFile = fs::path(__FILE__);
  const std::vector<fs::path> candidates = {
      fs::path("libsrc/App.cpp"),
      fs::path("../libsrc/App.cpp"),
      fs::path("../../libsrc/App.cpp"),
      thisFile.parent_path().parent_path() / "libsrc/App.cpp",
      fs::path("/home/ubuntu/00_work/04_wc/tmpl/libsrc/App.cpp"),
  };

  for (const auto &candidate : candidates) {
    std::ifstream input(candidate);
    if (input.good()) {
      std::ostringstream buffer;
      buffer << input.rdbuf();
      return buffer.str();
    }
  }

  ADD_FAILURE() << "Could not open libsrc/App.cpp from known locations";
  return {};
}

/**
 * @brief GUI 관련 프로젝트 소스 여러 파일을 합쳐 반환한다.
 *
 * App.cpp 외에 scene 구현 파일과 SceneUtil.cpp 에 분산된 정적 요구사항을 함께 검색할 때 사용한다.
 */
std::string LoadProjectUiSources() {
  namespace fs = std::filesystem;

  const fs::path thisFile = fs::path(__FILE__);
  const fs::path root = thisFile.parent_path().parent_path();
  const std::vector<fs::path> candidates = {
      root / "libsrc/App.cpp",
      root / "libsrc/scenes/SceneUtil.cpp",
      root / "libsrc/scenes/Scene_Title.cpp",
      root / "libsrc/scenes/Scene_MainMenu.cpp",
      root / "libsrc/scenes/Scene_Scenarios.cpp",
      root / "libsrc/scenes/Scene_Option.cpp",
      root / "libsrc/scenes/Scene_Play.cpp",
      root / "libsrc/scenes/Scene_End.cpp",
  };

  std::ostringstream buffer;
  for (const auto &candidate : candidates) {
    std::ifstream input(candidate);
    if (!input.good()) {
      ADD_FAILURE() << "Could not open required UI source file: " << candidate.string();
      return {};
    }
    buffer << "\n/* FILE: " << candidate.string() << " */\n";
    buffer << input.rdbuf();
  }
  return buffer.str();
}

/**
 * @brief 요구사항에 명시된 모든 씬과 전이 단계가 App.cpp 내에 선언되어 있는지 정적으로 검증한다.
 */
TEST(GuiStaticRequirementTest, DeclaresAllRequiredScenesAndTransitionPhases) {
  const std::string source = LoadAppCpp();
  ASSERT_FALSE(source.empty());

  EXPECT_NE(std::string::npos, source.find("TransitionPhase::TearingDown"));
  EXPECT_NE(std::string::npos, source.find("TransitionPhase::Loading"));
  EXPECT_EQ(std::string::npos, source.find("TRANSITION_DURATION_MS = 500"));
  EXPECT_NE(std::string::npos, source.find("phaseWorkDone"));
  EXPECT_NE(std::string::npos, source.find("phaseScreenPresented"));
  EXPECT_TRUE(source.find("RenderTransitionScreen(runtime)") != std::string::npos ||
              source.find("RenderTransitionScreen(ECS, runtime)") != std::string::npos);
  EXPECT_NE(std::string::npos, source.find("ToString(runtime.phase)"));
  EXPECT_EQ(std::string::npos, source.find("(now - runtime.phaseStartTicks) < TRANSITION_DURATION_MS"));
}

/**
 * @brief 전환 작업(OnExit/OnEnter)이 phase 내부에서 수행되도록 상태 머신이 구성되어 있는지 정적으로 검증한다.
 */
TEST(GuiStaticRequirementTest, EncodesPhaseDrivenTearDownAndLoadingWork) {
  const std::string source = LoadAppCpp();
  ASSERT_FALSE(source.empty());

  EXPECT_EQ(std::string::npos, source.find("runtime.prevScene = runtime.activeScene;\n    InvokeOnExit(ECS, runtime.activeScene);"));
  EXPECT_EQ(std::string::npos, source.find("InvokeOnExit(ECS, runtime.activeScene);\n    runtime.pendingScene.reset();"));

  EXPECT_NE(std::string::npos, source.find("if (runtime.phase == TransitionPhase::TearingDown)"));
  EXPECT_TRUE(source.find("InvokeOnExit(ECS, runtime.activeScene)") != std::string::npos ||
              source.find("InvokeOnExit(ECS, scene)") != std::string::npos);
  EXPECT_NE(std::string::npos, source.find("runtime.phase = TransitionPhase::Loading;"));
  EXPECT_NE(std::string::npos, source.find("if (runtime.phase == TransitionPhase::Loading)"));
  EXPECT_TRUE(source.find("InvokeOnEnter(ECS, *runtime.pendingScene)") != std::string::npos ||
              source.find("InvokeOnEnter(ECS, pending)") != std::string::npos);
}

/**
 * @brief 배경색 정책 및 종료 팝업 제어 흐름이 소스 코드에 올바르게 포함되어 있는지 검증한다.
 */
TEST(GuiStaticRequirementTest, EncodesBackgroundPolicyAndQuitPopupFlow) {
  const std::string source = LoadProjectUiSources();
  ASSERT_FALSE(source.empty());

  EXPECT_NE(std::string::npos, source.find("kSceneBackground{110, 110, 110}"));
  EXPECT_NE(std::string::npos, source.find("kTransitionBackground{60, 60, 60}"));
  EXPECT_NE(std::string::npos, source.find("ImGui::BeginPopupModal(\"Quit Confirmation\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"Yes\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"No\""));
  EXPECT_NE(std::string::npos, source.find("dispatcher.enqueue<AppQuitRequest>(AppQuitRequest{})"));
  EXPECT_NE(std::string::npos, source.find("runtime.exitAfterFinishing = true;"));
}

/**
 * @brief 라이프사이클 이벤트, 디스패처, 컨텍스트 등록 및 필수 ImGui UI 요소가 구현되어 있는지 검증한다.
 */
TEST(GuiStaticRequirementTest, EncodesLifecycleDispatcherCtxAndMinimumImGuiUi) {
  const std::string source = LoadProjectUiSources();
  ASSERT_FALSE(source.empty());

  EXPECT_NE(std::string::npos, source.find("m_pECS->ctx().emplace<AppCtx>();"));
  EXPECT_NE(std::string::npos, source.find("m_pECS->ctx().emplace<entt::dispatcher>();"));
  EXPECT_NE(std::string::npos, source.find("m_pECS->ctx().emplace<SceneRuntime>();"));
  EXPECT_NE(std::string::npos, source.find("dispatcher.sink<SceneTransitionRequest>().connect"));
  EXPECT_NE(std::string::npos, source.find("dispatcher.sink<AppQuitRequest>().connect"));
  EXPECT_NE(std::string::npos, source.find("OnGenericSceneEnter"));
  EXPECT_NE(std::string::npos, source.find("OnGenericSceneExit"));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"Start to Main Menu\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"Start Scenario\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"Option\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"Quit\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"Game Over\""));
  EXPECT_NE(std::string::npos, source.find("ImGui::Button(\"To Main Menu\""));
}

}  // namespace
