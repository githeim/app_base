# 테스트 검증 리포트

## 1. 검증 개요
- 프로젝트 경로: `/home/ubuntu/00_work/04_wc/tmpl`
- 공식 프로젝트명: `app_base`
- 테스트 대상 바이너리: `./build/Debug/test_app_base-1.0.out`
- 검증 시각: `2026-05-14T00:25:48+00:00`
- 목적: F-003 GUI 요구사항 검증 미완료 항목에 대해, headless 환경에서 자동 확인 가능한 범위를 최대한 보강한다.

## 2. 이번 보강 사항
- 신규 정적 검토형 테스트 파일 추가:
  - `test/testcase_02_gui_static.cpp`
- 신규 테스트 3건 추가:
  - `GuiStaticRequirementTest.DeclaresAllRequiredScenesAndTransitionPhases`
  - `GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`
  - `GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi`
- 접근 방식:
  - 실제 GUI 클릭 자동화 대신 `libsrc/App.cpp` 소스를 읽어 요구사항 관련 상수, scene 구성, lifecycle 연결, popup 흐름, EnTT `ctx`/`dispatcher`, ImGui 버튼 정의가 코드에 존재하는지 확인한다.
- 해석 주의:
  - 이 테스트들은 “코드상 구현 존재”를 자동 검증한다.
  - 실제 사람이 본 화면과 동일하게 보이는지는 검증하지 않는다.

## 3. 실행 명령
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
./build/Debug/test_app_base-1.0.out --gtest_list_tests
./build/Debug/test_app_base-1.0.out
```

## 4. 빌드 결과
### 4.1 Configure
- 결과: 성공
- 주요 출력:
  - `-- Configuring done (0.2s)`
  - `-- Generating done (0.0s)`
  - `-- Build files have been written to: /home/ubuntu/00_work/04_wc/tmpl/build`

### 4.2 Build
- 결과: 성공
- 실제 재컴파일 확인:
  - `Building CXX object CMakeFiles/test_app_base-1.0.out.dir/test/testcase_02_gui_static.cpp.o`
  - `Linking CXX executable Debug/test_app_base-1.0.out`
- 해석:
  - 신규 GUI 정적 검토 테스트가 실제로 빌드에 포함되었다.

## 5. 테스트 목록 확인 결과
명령:
```bash
./build/Debug/test_app_base-1.0.out --gtest_list_tests
```

결과: 성공

검출된 테스트:
- `CBaseDeathTest.StartWithoutMandatoryLoopProcedureTerminates`
- `SampleTest.BasicTest00`
- `RuntimeHelperTest.FpsCtrlRejectsInvalidArguments`
- `RuntimeHelperTest.FpsCtrlProducesPositiveDeltaAfterInitialization`
- `CBaseTest.ExecutesRegisteredProceduresInOrder`
- `GuiStaticRequirementTest.DeclaresAllRequiredScenesAndTransitionPhases`
- `GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`
- `GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi`

총계:
- 테스트 스위트: 5개
- 테스트 케이스: 8개

## 6. 전체 테스트 실행 결과
명령:
```bash
./build/Debug/test_app_base-1.0.out
```

결과: 성공

요약 출력:
- `[==========] Running 8 tests from 5 test suites.`
- `[==========] 8 tests from 5 test suites ran. (150 ms total)`
- `[  PASSED  ] 8 tests.`

세부 결과:
- 성공: 8
- 실패: 0
- 소요 시간: 150 ms

## 7. GUI 요구사항 관점의 자동 검증 근거
### 7.1 Scene / transition / lifecycle 존재 확인
`GuiStaticRequirementTest.DeclaresAllRequiredScenesAndTransitionPhases`는 다음 코드 존재를 자동 확인한다.
- scene enum:
  - `SceneId::Title`, `MainMenu`, `Scenarios`, `Option`, `Play`, `End`
- 전환 phase:
  - `TransitionPhase::FinishingUp`
  - `TransitionPhase::Preparing`
- 전환 시간 상수:
  - `TRANSITION_DURATION_MS = 2000`
- 전환 텍스트:
  - `"Finishing Up"`
  - `"Preparing"`

관련 코드 근거:
- `libsrc/App.cpp:20`
- `libsrc/App.cpp:31-44`
- `libsrc/App.cpp:67-95`
- `libsrc/App.cpp:235-273`
- `libsrc/App.cpp:448-457`

### 7.2 배경색 / Quit popup 흐름 확인
`GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`는 다음 코드 존재를 자동 확인한다.
- 일반 scene 배경색 상수:
  - `kSceneBackground{110, 110, 110}`
- transition 배경색 상수:
  - `kTransitionBackground{210, 235, 210}`
- Quit popup modal:
  - `ImGui::BeginPopupModal("Quit Confirmation" ... )`
- popup 버튼:
  - `Yes`
  - `No`
- Quit 요청 enqueue:
  - `dispatcher.enqueue<AppQuitRequest>(AppQuitRequest{})`
- Quit 확정 후 `Finishing Up` 흐름 진입 플래그:
  - `runtime.exitAfterFinishing = true;`

관련 코드 근거:
- `libsrc/App.cpp:28-29`
- `libsrc/App.cpp:359-367`
- `libsrc/App.cpp:421-432`
- `libsrc/App.cpp:501-513`

### 7.3 EnTT ctx / dispatcher / 최소 ImGui UI 확인
`GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi`는 다음 코드 존재를 자동 확인한다.
- EnTT ctx emplace:
  - `AppCtx`
  - `entt::dispatcher`
  - `SceneRuntime`
- dispatcher sink 연결:
  - `SceneTransitionRequest`
  - `AppQuitRequest`
- lifecycle handler:
  - `OnGenericSceneEnter`
  - `OnGenericSceneExit`
- 최소 ImGui 버튼 UI:
  - `Start to Main Menu`
  - `Start Scenario`
  - `Option`
  - `Quit`
  - `Game Over`
  - `To Main Menu`

관련 코드 근거:
- `libsrc/App.cpp:122-145`
- `libsrc/App.cpp:476-569`

## 8. 기존 테스트와 신규 테스트의 역할 분리
- 기존 테스트 5건:
  - 런타임 헬퍼, `CBase` 실행 순서, death test, 샘플 라이브러리 함수 검증
- 신규 테스트 3건:
  - GUI 요구사항의 “코드상 구현 존재”와 “구성 누락 여부”를 비침습적으로 검증
- 종합 해석:
  - 자동 테스트 범위가 GUI 비의존 로직 중심에서, GUI 요구사항 정적 근거 검증까지 확장되었다.
  - 다만 여전히 렌더링 결과의 시각적 적합성은 자동 테스트 범위 밖이다.

## 9. 경고 및 특이사항
- 다음 로그가 1회 출력됨:
  - `WARNING: Logging before InitGoogleLogging() is written to STDERR`
- 발생 구간:
  - `CBaseTest.ExecutesRegisteredProceduresInOrder`
- 영향도:
  - 테스트 실패로 이어지지 않았음
  - glog 초기화 이전 로깅 경고로 보이며, 기능 검증 결과 자체에는 영향 없음

## 10. 자동 검증 가능 범위와 한계
### 10.1 이번 자동 검증으로 강화된 범위
- 6개 본 scene가 코드에 모두 선언되고 scene map에 연결되어 있는지
- `Finishing Up` / `Preparing` phase와 2초 상수가 코드에 존재하는지
- 일반 scene / transition 배경색 상수가 분리되어 있는지
- Quit popup 이 `Yes` / `No` modal 구조인지
- Quit 확정 시 즉시 종료가 아니라 `exitAfterFinishing` 경로를 사용하는지
- EnTT `ctx` / `dispatcher` 사용 흔적이 실제 코드에 존재하는지
- ImGui 최소 버튼 UI 문자열이 코드에 포함되어 있는지

### 10.2 이번 자동 검증으로도 확인하지 못한 범위
- 버튼 클릭 시 실제 프레임에서 원하는 장면으로 전환되는지
- `Finishing Up` 2초 후 `Preparing` 2초가 실제로 체감되는지
- 회색 / 옅은 녹색 배경이 실제 렌더링 결과에서 올바르게 보이는지
- Quit popup 이 화면에 시각적으로 뜨고 포커스/입력이 자연스러운지
- 실제 디스플레이에서 레이아웃, 폰트, 버튼 위치가 적절한지

## 11. 결론
- 자동 테스트 검증 결과: 성공
- 전체 테스트 상태: 8/8 통과
- F-003 관점 성과:
  - GUI 요구사항의 일부를 새 정적 검토형 테스트로 자동 검증 가능한 상태까지 보강했다.
  - 다만 이는 “코드상 근거 강화”이며 “실제 GUI 시각 검증 완료”와는 다르다.
- 가장 정확한 현재 표현:
  - `GUI 요구사항은 코드상/자동 테스트상 일부 보강 확인되었으나, 시각적 최종 검증은 여전히 수동 확인이 필요하다.`
