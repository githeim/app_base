#include <gtest/gtest.h>


/**
 * @brief GoogleTest 실행을 위한 테스트 전용 진입점이다.
 *
 * @param[in] argc 명령행 인자 개수
 * @param[in] argv 명령행 인자 목록
 * @return 전체 테스트 실행 결과 코드
 */
int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc,(char**)argv);
  return RUN_ALL_TESTS();
}
