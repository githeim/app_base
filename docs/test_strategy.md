# Test Strategy Document — app_base

## Document Revision

| Version | Date       | Comment                                           | Author |
|---------|------------|---------------------------------------------------|--------|
| 0.1     | 2026-05-13 | 초기 테스트 전략 및 실행 결과 반영               | tester |
| 0.2     | 2026-05-14 | GUI 정적 검토 테스트 3건 및 자동 검증 범위 확대 반영 | tester |
| 0.3     | 2026-05-14 | Scene 파일 분리, SceneDef.h, GameState, onUpdate 반영 | tester |

---

## 1. 문서 목적

이 문서는 `/home/ubuntu/00_work/04_wc/tmpl` 프로젝트의 테스트 접근 방식을 정의한다. SDL2 + EnTT + ImGui 기반 scene 전이 템플릿의 특성을 고려하여 단위 테스트, 정적 검토 테스트, 통합 테스트, 시나리오 테스트의 범위를 구분하고, 요구사항 ID(FR/NFR)와 테스트 케이스 ID의 추적 관계를 명시한다. 또한 현재 headless/Docker/Xvfb 환경 제약과 GUI 중심 구조 때문에 자동화 가능한 항목과 수동 확인이 필요한 항목을 구분한다.

## 2. 테스트 범위와 기본 원칙

### 2.1 테스트 범위
- 포함 범위
  - scene 전이 구조의 핵심 규칙 검증
  - 런타임 보조 로직(FPS 제어, loop 실행 순서) 검증
  - `libsrc/App.cpp` 및 `libsrc/scenes/Scene_*.cpp`에 구현된 GUI 요구사항의 코드상 존재 여부 정적 검토
  - `SceneDef.h`의 타입 정의(SceneId, PlayArgs, GameState 등) 존재 여부 정적 검토
  - 빌드 및 테스트 실행 가능 상태 검증
  - 문서 요구사항 대비 자동화/수동 검증 경계 정의
- 제외 범위
  - 실제 게임 플레이 로직 검증
  - 실제 사용자 입력 자동화 기반 GUI E2E
  - 실제 오디오 장치/디스플레이 장치에 강하게 의존하는 검증

### 2.2 기본 원칙
- GUI 없이 검증 가능한 로직부터 자동화한다.
- GUI 렌더링 결과 자체는 수동 확인이 필요하더라도, scene 선언/전이 단계/배경 정책/popup/UI 구성요소의 코드상 존재는 정적 검토 테스트로 우선 자동화한다.
- SDL 실제 창 생성, ImGui 렌더링 결과 픽셀 비교, 오디오 장치 의존 검증은 현재 환경에서 무리하게 자동화하지 않는다.
- 기존 `test/` 구조와 CMake 방식을 유지한다.
- 요구사항 추적 가능성을 위해 모든 테스트 케이스에 ID를 부여한다.

## 3. 테스트 대상 특성 및 환경 제약

[표 3-1]

| 항목 | 내용 | 테스트 영향 |
|------|------|-------------|
| 실행 구조 | SDL2 + EnTT + ImGui 기반 앱 루프 | 로직과 GUI를 분리해 테스트해야 효율적이다. |
| scene 전이 | dispatcher 이벤트와 transition state로 제어 | 전이 규칙 자체는 로직 관점에서 검증 가능하나, 실제 화면 노출은 GUI 확인이 필요하다. |
| GUI 프레임워크 | ImGui + SDL Renderer | 버튼 클릭/팝업/텍스트 노출은 실제 프레임 렌더링 확인이 유리하다. |
| 실행 환경 | headless/Docker/Xvfb 가능 환경 | 가상 디스플레이 없이는 창 생성 기반 테스트가 불안정할 수 있다. |
| 오디오 | SDL_mixer 초기화 시 실제 오디오 장치 영향 가능 | 자동 테스트에서 오디오 장치 의존 검증은 회피하는 편이 안전하다. |

### 3.1 현재 자동화 제약
- headless 또는 Docker 환경에서는 SDL 윈도우 생성이 실패하거나, Xvfb 유무에 따라 결과가 달라질 수 있다.
- ImGui 버튼 클릭, 팝업 표시, 색상 차이, 텍스트 표시 여부는 프레임 버퍼 캡처 또는 UI 자동화 도구 없이는 신뢰성 있게 검증하기 어렵다.
- `Loading`, `TearingDown`은 phase 내부 작업 전에 먼저 화면이 떠야 하므로, “최소 1프레임 먼저 보였는지”와 “phase 내부 작업이 실제로 그 단계에서 수행되는지”를 함께 확인해야 한다.
- 오디오 초기화는 장치 존재 여부에 따라 경고 또는 실패 양상이 달라질 수 있어 CI 안정성이 낮다.
- 따라서 현재 자동화는 비GUI 로직 검증에 더해, `test/testcase_02_gui_static.cpp`를 통한 GUI 요구사항의 정적 존재 확인까지 확대된 상태다.

## 4. 테스트 수준별 전략

### 4.1 단위 테스트
- 대상
  - `FPS_Ctrl`, `Init_FPS_Ctrl`
  - `CBase`의 procedure 실행 순서
  - `CBase`의 loop procedure 필수 조건
  - 단순 라이브러리 함수(`testmodule_Test`)
- 목적
  - GUI 없이도 회귀 검증 가능한 기반 로직을 빠르게 확인한다.
- 자동화 여부
  - 자동화 가능

### 4.2 정적 검토 테스트
- 대상
  - `test/testcase_02_gui_static.cpp`
  - scene ID 선언, transition phase, 전환 문구/phase 진행 플래그
  - 배경색 정책, Quit popup 흐름, lifecycle hook/dispatcher/ctx 저장
  - 최소 ImGui 버튼 UI 구성요소 존재 여부
  - `SceneDef.h`의 GameState, PlayArgs 타입 선언 여부
  - `libsrc/scenes/Scene_*.cpp` 파일 구조 및 onUpdate 훅 존재 여부
- 목적
  - headless 환경에서도 GUI 요구사항이 코드에 반영되어 있는지 자동 확인한다.
  - 기존 비GUI 로직 중심 자동 검증 범위를 GUI 요구사항의 코드상 존재 확인까지 확대한다.
- 자동화 여부
  - 자동화 가능
  - 단, 실제 렌더링 결과와 사용자 체감은 대체하지 못한다.

### 4.3 통합 테스트
- 대상
  - 테스트 바이너리 링크 상태
  - 기존 라이브러리와 테스트 코드의 결합
  - 빌드 산출물 생성 및 gtest 실행
- 목적
  - 템플릿이 실제로 빌드되고 테스트 바이너리가 실행 가능한지 확인한다.
- 자동화 여부
  - 자동화 가능

### 4.4 시나리오 테스트
- 대상
  - Title → Main Menu
  - Main Menu → Scenarios / Option / Quit popup
  - Scenarios → Play → End → Main Menu
  - Quit popup의 Yes/No 흐름
  - `TearingDown`, `Loading` 선표시 및 phase 내부 작업, 배경색 구분
- 목적
  - 사용자가 실제 화면에서 경험하는 전이 흐름을 검증한다.
- 자동화 여부
  - 현재는 주로 수동 검증

## 5. 테스트 케이스 목록

[표 5-1]

| 테스트 케이스 ID | 수준 | 자동화 | 구현 상태 | 검증 대상 | 비고 |
|------------------|------|--------|-----------|-----------|------|
| TC-UNIT-001 | 단위 | 가능 | 구현 완료 | `testmodule_Test()` 반환값 | 기존 테스트 유지 |
| TC-UNIT-002 | 단위 | 가능 | 구현 완료 | `FPS_Ctrl`가 잘못된 인자를 거부하는지 | `test/testcase_01_runtime.cpp` |
| TC-UNIT-003 | 단위 | 가능 | 구현 완료 | `Init_FPS_Ctrl` 이후 `FPS_Ctrl`가 양수 FPS/시간차를 반환하는지 | `test/testcase_01_runtime.cpp` |
| TC-UNIT-004 | 단위 | 가능 | 구현 완료 | `CBase`가 pre → loop → post 순으로 procedure를 실행하는지 | `test/testcase_01_runtime.cpp` |
| TC-UNIT-005 | 단위 | 가능 | 구현 완료 | `CBase`가 loop procedure 없이 시작되면 fatal 종료되는지 | `test/testcase_01_runtime.cpp`, death test |
| TC-STAT-001 | 정적 검토 | 가능 | 구현 완료 | transition phase 2종, phase 진행 플래그, 전환 화면 렌더 경로 선언 여부 | `test/testcase_02_gui_static.cpp`, `GuiStaticRequirementTest.DeclaresAllRequiredScenesAndTransitionPhases` |
| TC-STAT-002 | 정적 검토 | 가능 | 구현 완료 | 본 scene/transition 배경 정책과 Quit popup Yes/No 종료 흐름 인코딩 여부 | `test/testcase_02_gui_static.cpp`, `GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow` |
| TC-STAT-003 | 정적 검토 | 가능 | 구현 완료 | EnTT ctx/dispatcher/lifecycle hook 및 최소 ImGui UI 구성요소 존재 여부 | `test/testcase_02_gui_static.cpp`, `GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi` |
| TC-INT-001 | 통합 | 가능 | 실행 완료 | Debug 테스트 바이너리 빌드 성공 | CMake + gtest |
| TC-INT-002 | 통합 | 가능 | 실행 완료 | 테스트 바이너리 전체 실행 성공 | `build/Debug/test_app_base-1.0.out` |
| TC-SCN-001 | 시나리오 | 제한적 | 수동 대상 | Title에서 Main Menu로 이동 | 버튼 클릭 필요 |
| TC-SCN-002 | 시나리오 | 제한적 | 수동 대상 | Main Menu에서 Scenarios/Option 이동 | GUI 전이 확인 |
| TC-SCN-003 | 시나리오 | 제한적 | 수동 대상 | Scenarios에서 Scenario 1~4가 Play로 이동 | PlayArgs.selectedScenario HUD 표시 포함 |
| TC-SCN-004 | 시나리오 | 제한적 | 수동 대상 | Play에서 삼각형/원/사각형 애니메이션, HUD(GameState) 표시 확인 | onUpdate 동작 시각 확인 |
| TC-SCN-004b | 시나리오 | 제한적 | 수동 대상 | Play에서 Game Over 후 End 이동 | GUI 전이 확인 |
| TC-SCN-005 | 시나리오 | 제한적 | 수동 대상 | End에서 Main Menu 복귀 | GUI 전이 확인 |
| TC-SCN-006 | 시나리오 | 제한적 | 수동 대상 | Quit popup의 No 동작 | modal UI 확인 필요 |
| TC-SCN-007 | 시나리오 | 제한적 | 수동 대상 | Quit popup의 Yes 후 종료 흐름 | `TearingDown` 표시 포함 |
| TC-SCN-008 | 시나리오 | 불가에 가까움 | 수동 대상 | 본 scene 회색 / transition 옅은 녹색 확인 | 색상 시각 확인 필요 |
| TC-SCN-009 | 시나리오 | 제한적 | 수동 대상 | `TearingDown`/`Loading` 선표시와 즉시 단계 전환 확인 | 실제 화면 확인이 적합 |

## 6. 요구사항 추적 매핑

[표 6-1]

| 요구사항 ID | 설명 요약 | 대응 테스트 케이스 |
|-------------|-----------|--------------------|
| FR-001 | SDL2 기반 윈도우/메인 루프 | TC-INT-001, TC-INT-002, TC-SCN-001~009 |
| FR-002 | SDL runtime context를 EnTT ctx에 저장 | TC-STAT-003, TC-SCN-001~009 |
| FR-003 | dispatcher 기반 scene 전이 | TC-STAT-001, TC-STAT-003, TC-SCN-001~007 |
| FR-004 | 6개 본 scene 제공 | TC-STAT-001, TC-SCN-001~005 |
| FR-005 | 최초 활성 scene은 Title | TC-SCN-001 |
| FR-006 | Title scene UI와 Main Menu 이동 | TC-STAT-003, TC-SCN-001 |
| FR-007 | Main Menu UI와 Quit popup | TC-STAT-002, TC-STAT-003, TC-SCN-002, TC-SCN-006, TC-SCN-007 |
| FR-008 | Scenarios UI와 Play 이동 | TC-STAT-003, TC-SCN-003 |
| FR-009 | Option UI와 Main Menu 복귀 | TC-SCN-002 |
| FR-010 | Play placeholder와 End 이동 | TC-STAT-003, TC-SCN-003, TC-SCN-004 |
| FR-011 | End UI와 Main Menu 복귀 | TC-STAT-003, TC-SCN-005 |
| FR-012 | scene별 OnEnter/OnExit | TC-STAT-003, TC-SCN-001~007 |
| FR-013 | TearingDown 화면 표시 → OnExit → Loading 화면 표시 → OnEnter → 본 화면 | TC-STAT-001, TC-SCN-001~007, TC-SCN-009 |
| FR-014 | Quit popup Yes/No | TC-STAT-002, TC-SCN-006, TC-SCN-007 |
| FR-015 | Quit 시 TearingDown 후 종료 | TC-STAT-002, TC-SCN-007 |
| FR-016 | 본 scene 회색 배경 | TC-STAT-002, TC-SCN-008 |
| FR-017 | transition 텍스트/옅은 녹색 배경 | TC-STAT-001, TC-STAT-002, TC-SCN-008, TC-SCN-009 |
| FR-018 | ImGui 최소 버튼/텍스트 UI | TC-STAT-003, TC-SCN-001~007 |
| FR-019 | 기존 윈도우 기본값 유지 | TC-SCN-001 |
| NFR-001 | 기존 구조 유지 | TC-INT-001, TC-INT-002 |
| NFR-002 | 가독성 있는 scene 전이 구조 | TC-STAT-001~003, TC-SCN-001~009 |
| NFR-003 | 외부 설정 파일 미사용 | 정적 코드 리뷰 + TC-INT-001 |
| NFR-004 | 재사용 가능한 템플릿 | TC-INT-001, TC-INT-002 |
| NFR-005 | 실제 게임플레이 미포함 | TC-SCN-003, TC-SCN-004 |
| NFR-006 | 빌드 가능 상태 유지 | TC-INT-001 |
| NFR-007 | 테스트 가능 상태 유지 | TC-INT-002, TC-UNIT-001~005, TC-STAT-001~003 |
| NFR-008 | 실행 검증 가능 상태 유지 | TC-INT-002, TC-SCN-001~009 |
| NFR-009 | 문서-구현 일관성 | 본 문서 + 코드/실행 결과 검토 |

## 7. 자동화 가능/불가능 항목 정리

[표 7-1]

| 구분 | 항목 | 자동화 가능 여부 | 이유 |
|------|------|------------------|------|
| 로직 | FPS 제어 인자 검증 | 가능 | GUI/장치 의존성이 없다. |
| 로직 | loop procedure 순서 | 가능 | 순수 C++ 동작으로 검증 가능하다. |
| 로직 | loop procedure 필수 여부 | 가능 | 예외적 시작 조건을 단위 테스트로 검증 가능하다. |
| 정적 검토 | transition phase/phase 진행 플래그/전환 순서 존재 | 가능 | 소스 코드 문자열 검증으로 headless 환경에서도 안정적으로 확인 가능하다. |
| 정적 검토 | 배경색 정책과 Quit popup 종료 흐름 존재 | 가능 | 실제 렌더링 전이라도 정책과 분기 구현 존재를 자동 확인할 수 있다. |
| 정적 검토 | ctx/dispatcher/lifecycle hook/최소 ImGui UI 존재 | 가능 | GUI 요구사항의 구조적 반영 여부를 코드 수준에서 검증 가능하다. |
| 빌드 | 테스트 바이너리 생성 | 가능 | CMake 빌드 결과로 명확히 판단 가능하다. |
| 실행 | gtest 전체 실행 | 가능 | 현재 `./build/Debug/test_app_base-1.0.out` 기준 5개 suite, 8개 테스트가 headless에서 통과한다. |
| GUI | ImGui 버튼 클릭 결과 | 제한적 | 입력 주입 및 프레임 동기화 장치가 없다. |
| GUI | popup 표시 여부 | 제한적 | modal UI는 실제 프레임 확인이 필요하다. |
| GUI | 회색/옅은 녹색 배경 정책 | 사실상 수동 | 픽셀 캡처/비교 도구가 없고 시각 확인이 가장 단순하다. |
| GUI | 전환 화면 선표시 + phase 내부 작업 체감 | 제한적 | 내부 상태 검증과 실제 체감 검증이 다르다. |
| 오디오 | 실제 사운드 재생 | 권장하지 않음 | 오디오 장치 유무에 따라 CI 안정성이 낮다. |

## 8. 실제 실행 절차

### 8.1 빌드
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- `cmake --build build -j`

### 8.2 자동 테스트 실행
- `./build/Debug/test_app_base-1.0.out --gtest_list_tests`
- `./build/Debug/test_app_base-1.0.out`
- 현재 확인 결과: `GuiStaticRequirementTest` 4건을 포함한 전체 테스트가 통과한다.

### 8.3 수동 시나리오 확인 권장 절차
- Xvfb 또는 실제 디스플레이 환경에서 `./build/Debug/app_base-1.0.out` 실행
- 아래 순서로 확인
  1. 최초 Title 노출
  2. Main Menu 이동
  3. Scenarios 이동 후 Scenario 1~4 중 임의 선택
  4. Play에서 HUD에 선택값(selectedScenario), 씬 점수, GameState(playerName/level/gold/playTimeSec) 표시 확인
  5. Play에서 삼각형 회전, 원 맥박, 사각형 고정 애니메이션 확인
  6. Play에서 `Game Over` 클릭 후 End 이동
  7. End에서 Main Menu 복귀 확인
  8. Main Menu의 Quit → No 확인
  9. Main Menu의 Quit → Yes 확인
  10. 전환 시 `TearingDown`, `Loading` 및 배경색 확인

## 9. 이번 보강 결과 요약
- 기존 샘플 테스트와 런타임 보조 로직 테스트에 더해, `test/testcase_02_gui_static.cpp` 기반 GUI 정적 검토 테스트 3건을 명시적으로 반영했다.
- 자동 검증 범위는 기존의 비GUI 로직 + 빌드/실행 가능성 확인에서, scene 선언/transition phase/배경 정책/Quit popup/ctx·dispatcher·최소 ImGui UI의 코드상 존재 확인까지 확대되었다.
- 현재 자동 테스트 실행 기준은 `./build/Debug/test_app_base-1.0.out` 전체 통과이다.
- 다만 scene 버튼의 실제 반응, popup의 실제 표시, 전환 화면 체감 시간, 회색/옅은 녹색 배경의 시각 검증은 계속 수동/후속 자동화 대상이다.

## 10. 향후 개선 방향
- scene 전이 상태 머신을 별도 클래스/헤더로 분리하면 dispatcher/transition 로직의 자동 단위 테스트 범위를 넓힐 수 있다.
- SDL dummy video driver 또는 Xvfb를 명시적으로 사용하는 통합 테스트 harness를 추가하면 창 생성 기반 smoke test를 안정화할 수 있다.
- 필요 시 프레임 버퍼 캡처 기반 스냅샷 검증을 도입해 배경색/텍스트 노출 자동화를 점진적으로 확대할 수 있다.
