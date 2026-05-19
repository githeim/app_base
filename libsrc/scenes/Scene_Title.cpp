#include "Scene_Title.h"
#include "SceneUtil.h"
#include "SceneDef.h"
#include "imgui.h"

namespace Scene::Title {

/**
 * @brief Title 씬 진입 시 호출된다.
 *
 * 공통 진입 카운터를 갱신한다.
 * Title 씬은 앱 최초 부팅 시 Preparing 단계 없이 즉시 표시된다.
 *
 * @param[in,out] ECS ECS 레지스트리
 */
void OnEnter(entt::registry &ECS) {
  OnGenericSceneEnter(ECS, SceneId::Title);
}

/**
 * @brief Title 씬 이탈 시 호출된다. 공통 이탈 카운터를 갱신한다.
 *
 * @param[in,out] ECS ECS 레지스트리
 */
void OnExit(entt::registry &ECS) {
  OnGenericSceneExit(ECS, SceneId::Title);
}

/**
 * @brief 매 프레임 Title 씬 UI를 렌더링한다.
 *
 * "Start to Main Menu" 버튼 클릭 시 MainMenu 씬으로의 전이를 요청한다.
 *
 * @param[in,out] ECS ECS 레지스트리
 */
void OnRender(entt::registry &ECS) {
  RenderSceneChrome(ECS, SceneId::Title, "Title",
                   "앱 최초 부팅 시에는 별도 Preparing 없이 Title 씬으로 즉시 진입한다.");

  auto &dispatcher = ECS.ctx().get<entt::dispatcher>();
  if (ImGui::Button("Start to Main Menu", ImVec2(260.0f, 40.0f))) {
    dispatcher.enqueue<SceneTransitionRequest>(SceneTransitionRequest{SceneId::MainMenu});
  }

  EndFullscreenUi();
}

}  // namespace Scene::Title
