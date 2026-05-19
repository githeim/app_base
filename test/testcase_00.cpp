#include <gtest/gtest.h>
#include "libmodule.h"


// Test template    

/**
 * @brief 기본 예제 테스트를 위한 테스트 fixture 클래스다.
 */
class SampleTest : public testing::Test {
  protected:
    /**
     * @brief 각 테스트 시작 전에 필요한 공통 준비 작업을 수행한다.
     */
    virtual void SetUp() {
    }
    /**
     * @brief 각 테스트 종료 후 공통 정리 작업을 수행한다.
     */
    virtual void TearDown() {
    }
};
 
/**
 * @brief 기본 산술 연산 및 모듈 함수가 정상적으로 값을 반환하는지 테스트한다.
 */
TEST_F(SampleTest, BasicTest00) {
  EXPECT_EQ(2, (1+1));
  EXPECT_EQ(5, (2+3));
  EXPECT_EQ(testmodule_Test(), 7);
}


