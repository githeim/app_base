#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "CBase.h"

namespace {

/**
 * @brief 잘못된 인자가 주어졌을 때 FPS 제어기가 이를 거부하고 0을 반환하는지 검증한다.
 */
TEST(RuntimeHelperTest, FpsCtrlRejectsInvalidArguments) {
  double diffSec = -1.0;
  EXPECT_DOUBLE_EQ(0.0, FPS_Ctrl(static_cast<FPS_Idx_T>(-1), 1.0 / 60.0, diffSec));
  EXPECT_DOUBLE_EQ(0.0, diffSec);

  diffSec = -1.0;
  EXPECT_DOUBLE_EQ(0.0, FPS_Ctrl(MAIN_APP, 0.0, diffSec));
  EXPECT_DOUBLE_EQ(0.0, diffSec);
}

/**
 * @brief 초기화 후 정상적인 FPS 측정 시 양수의 델타 시간과 FPS가 도출되는지 검증한다.
 */
TEST(RuntimeHelperTest, FpsCtrlProducesPositiveDeltaAfterInitialization) {
  Init_FPS_Ctrl();

  double diffSec = 0.0;
  const double actualFps = FPS_Ctrl(SCENE_CHECKER, 0.001, diffSec);

  EXPECT_GT(diffSec, 0.0);
  EXPECT_GT(actualFps, 0.0);
}

/**
 * @brief 등록된 전처리, 메인, 후처리 루프 프로시저가 올바른 순서대로 실행되는지 검증한다.
 */
TEST(CBaseTest, ExecutesRegisteredProceduresInOrder) {
  CBase base;
  entt::registry ecs;
  std::vector<std::string> calls;

  Proc_Map_T procMap = {
      {PRE_LOOP_PROC,
       [&calls](entt::registry &) {
         calls.emplace_back("pre");
         return 0;
       }},
      {LOOP_PROC,
       [&calls](entt::registry &) {
         calls.emplace_back("loop");
         return 1;
       }},
      {POST_LOOP_PROC,
       [&calls](entt::registry &) {
         calls.emplace_back("post");
         return 0;
       }},
  };

  ASSERT_EQ(0, base.Start(ecs, procMap));
  EXPECT_EQ(0, base.Wait());

  ASSERT_EQ(3u, calls.size());
  EXPECT_EQ("pre", calls[0]);
  EXPECT_EQ("loop", calls[1]);
  EXPECT_EQ("post", calls[2]);
}

/**
 * @brief 필수적인 메인 루프 프로시저 누락 시 프로그램이 예외 종료(Death)되는지 검증한다.
 */
TEST(CBaseDeathTest, StartWithoutMandatoryLoopProcedureTerminates) {
  const auto runWithoutLoopProc = []() {
    CBase base;
    entt::registry ecs;
    Proc_Map_T procMap;
    procMap.emplace(PRE_LOOP_PROC, [](entt::registry &) { return 0; });
    procMap.emplace(POST_LOOP_PROC, [](entt::registry &) { return 0; });
    (void)base.Start(ecs, procMap);
  };

  EXPECT_DEATH(runWithoutLoopProc(), "Loop procedure is mandatory");
}

}  // namespace
