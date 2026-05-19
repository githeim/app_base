# 요구사항 명세서

## 1. 문서 목적
본 문서는 `/home/ubuntu/00_work/04_wc/tmpl` 프로젝트를 SDL2 + EnTT + ImGui 기반의 scene 중심 앱 베이스 프레임워크 템플릿으로 정리하기 위한 구현 준비 수준의 요구사항을 정의한다. 모든 요구사항은 기능 요구사항(FR)과 비기능 요구사항(NFR)으로 분류하고, 요구사항별 출처와 의존 관계를 명시한다.

## 2. 원천 산출물
- IL-Q1: `docs/interview_log.md` 21-26행
- IL-Q2: `docs/interview_log.md` 28-38행
- IL-Q3: `docs/interview_log.md` 40-45행
- IL-Q4: `docs/interview_log.md` 46-54행
- IL-Q5: `docs/interview_log.md` 56-69행
- IL-Q6: `docs/interview_log.md` 71-74행
- IL-Q7: `docs/interview_log.md` 76-80행
- IL-Q8: `docs/interview_log.md` 82-87행
- IL-Q9: `docs/interview_log.md` 89-93행
- IL-Q10: `docs/interview_log.md` 95-100행
- IL-DI: `docs/interview_log.md` 102-107행
- BR-1: `docs/basic_requirements.md` 3-8행
- BR-2: `docs/basic_requirements.md` 9-13행
- BR-3: `docs/basic_requirements.md` 15-30행
- BR-4: `docs/basic_requirements.md` 32-40행
- BR-5: `docs/basic_requirements.md` 42-71행
- BR-6: `docs/basic_requirements.md` 73-81행
- BR-7: `docs/basic_requirements.md` 83-86행
- BR-8: `docs/basic_requirements.md` 88-92행
- BR-9: `docs/basic_requirements.md` 94-96행
- BR-10: `docs/basic_requirements.md` 98-103행

## 3. 전제 및 확인 필요 사항
- A-001 [추정]: 앱 시작 직후 최초 활성 본 씬은 `Title`로 본다. 명시적 부정 진술이 없고, scene 목록과 사용자 흐름이 Title에서 시작하는 구조이므로 기본 전제로 채택한다. 출처: IL-Q2, IL-Q3, BR-5.
- A-002 [추정]: `OnEnter`와 `OnExit` handler는 scene 객체 또는 scene 관리 계층의 명시적 훅으로 구현하고, 전환 화면은 먼저 표시한 뒤 phase 내부에서 handler를 수행하는 것으로 본다. 출처: IL-Q9, BR-6.
- C-001 [확인 필요]: 앱 최초 부팅 시 `Title` 진입 전에 별도 `Loading` phase를 둘지 여부는 추가 확인이 필요하다. 현재 구현은 앱 최초 진입에는 별도 transition phase를 두지 않는다. 출처: IL-Q9, BR-6.
- C-002 [확인 필요]: `Scenario 1~4` 선택값을 `Play` 씬 컨텍스트에 별도 전달하여 이후 확장 포인트로 유지할지 여부는 추가 확인이 필요하다. 현재 산출물은 4개 버튼이 모두 `Play`로 이동한다는 점만 명시한다. 출처: IL-Q5, BR-5.

## 4. 기능 요구사항 (FR)

### FR-001
- 요구사항: 시스템은 SDL2 기반으로 윈도우를 생성하고 메인 루프를 구동해야 한다.
- 세부 조건:
  - 애플리케이션 시작 시 SDL runtime을 초기화해야 한다.
  - 렌더링 및 이벤트 처리가 가능한 기본 윈도우 실행 상태를 확보해야 한다.
- 출처: IL-Q1, BR-4
- 의존: 없음

### FR-002
- 요구사항: 시스템은 SDL 관련 runtime context를 EnTT `ctx`에 저장하고 전역성 없는 방식으로 접근 가능해야 한다.
- 세부 조건:
  - 윈도우, 렌더러, ImGui 연동에 필요한 런타임 객체는 `entt::registry::ctx`를 통해 조회 가능해야 한다.
  - scene 및 앱 루프 계층은 필요한 공유 런타임 상태를 `ctx`를 통해 사용해야 한다.
- 출처: IL-Q1, IL-Q8, IL-DI, BR-2, BR-4
- 의존: FR-001

### FR-003
- 요구사항: 시스템은 scene 전이 요청을 EnTT `dispatcher` 기반 이벤트로 표현하고 처리해야 한다.
- 세부 조건:
  - 버튼 클릭 등 UI 입력은 직접 scene를 즉시 교체하기보다 전이 요청 이벤트를 발행하는 방식이어야 한다.
  - 전이 관리 계층은 dispatcher를 구독하여 현재 scene 종료, 전환 화면 표시, 다음 scene 진입을 순차 처리해야 한다.
- 출처: IL-Q8, IL-DI, BR-2, BR-4
- 의존: FR-002, FR-012, FR-013

### FR-004
- 요구사항: 시스템은 `Title`, `Main Menu`, `Scenarios`, `Option`, `Play`, `End`의 6개 본 scene를 제공해야 한다.
- 세부 조건:
  - 각 scene는 독립적인 렌더링과 입력 처리 단위를 가져야 한다.
  - scene 식별자는 코드에서 명시적으로 구분 가능해야 한다.
- 출처: IL-Q2, BR-3, BR-5
- 의존: FR-001, FR-002

### FR-005
- 요구사항: 시스템은 앱 시작 후 `Title` scene를 최초 활성 본 scene로 표시해야 한다. [추정]
- 세부 조건:
  - 사용자 최초 진입 흐름은 `Title`에서 시작해야 한다.
  - 다른 scene로의 최초 진입 구조를 두더라도 외부 설정 파일로 변경 가능하게 만들지 않아야 한다.
- 출처: IL-Q2, IL-Q3, BR-5, A-001
- 의존: FR-004

### FR-006
- 요구사항: 시스템은 `Title` scene에서 `Title` 텍스트와 `Start to Main Menu` 버튼을 표시해야 한다.
- 세부 조건:
  - 사용자가 `Start to Main Menu` 버튼을 누르면 `Main Menu`로 전이 요청을 발생시켜야 한다.
- 출처: IL-Q3, BR-5
- 의존: FR-003, FR-004, FR-005

### FR-007
- 요구사항: 시스템은 `Main Menu` scene에서 `Start Scenario`, `Option`, `Quit` 버튼을 표시해야 한다.
- 세부 조건:
  - `Start Scenario` 클릭 시 `Scenarios`로 전이 요청을 발생시켜야 한다.
  - `Option` 클릭 시 `Option`으로 전이 요청을 발생시켜야 한다.
  - `Quit` 클릭 시 종료 popup을 표시해야 하며 즉시 종료해서는 안 된다.
- 출처: IL-Q4, BR-5, BR-8
- 의존: FR-003, FR-004, FR-014

### FR-008
- 요구사항: 시스템은 `Scenarios` scene에서 `Scenario 1`, `Scenario 2`, `Scenario 3`, `Scenario 4`, `Back To Main Menu` 버튼을 표시해야 한다.
- 세부 조건:
  - `Scenario 1~4` 버튼은 현재 버전에서 모두 `Play` scene로 전이 요청을 발생시켜야 한다.
  - `Back To Main Menu` 버튼은 `Main Menu`로 전이 요청을 발생시켜야 한다.
- 출처: IL-Q5, BR-5
- 의존: FR-003, FR-004

### FR-009
- 요구사항: 시스템은 `Option` scene에서 `Back To Main Menu` 버튼 하나를 표시해야 한다.
- 세부 조건:
  - 버튼 클릭 시 `Main Menu`로 전이 요청을 발생시켜야 한다.
- 출처: IL-Q5, BR-5
- 의존: FR-003, FR-004

### FR-010
- 요구사항: 시스템은 `Play` scene에서 `Game Over` 버튼과 함께 ctx 기반 상태 관리 예시를 제공해야 한다.
- 세부 조건:
  - `Game Over` 버튼 클릭 시 `End` scene로 전이 요청을 발생시켜야 한다.
  - Play 씬은 상단 1/3 ImGui HUD, 하단 2/3 SDL Renderer 게임 드로잉 영역으로 구성한다.
  - ECS 컴포넌트(씬 내 일시 상태)와 `GameState` ctx(전역 누적 상태)의 사용 예시를 포함한다.
  - `onUpdate` 훅에서 삼각형 회전, 원 맥박, 점수 증가, playTimeSec 누적 처리를 예시로 보여준다.
  - 씬 재진입 시 Play 씬 전용 entity/컴포넌트는 재생성되고 `GameState`는 누적 유지된다.
- 출처: IL-Q5, BR-3, BR-5
- 의존: FR-003, FR-004

### FR-011
- 요구사항: 시스템은 `End` scene에서 `To Main Menu` 버튼 하나를 표시해야 한다.
- 세부 조건:
  - 버튼 클릭 시 `Main Menu`로 전이 요청을 발생시켜야 한다.
- 출처: IL-Q5, BR-5
- 의존: FR-003, FR-004

### FR-012
- 요구사항: 시스템은 각 본 scene에 대해 진입 시점 `OnEnter` handler와 탈출 시점 `OnExit` handler를 제공해야 한다.
- 세부 조건:
  - scene 변경 시 이전 scene의 `OnExit`가 먼저 실행되어야 한다.
  - 현재 scene의 `OnExit`는 `TearingDown` phase 내부에서 실행되어야 한다.
  - 대상 scene의 `OnEnter`는 `Loading` phase 내부에서 실행되어야 한다.
- 출처: IL-Q9, BR-3, BR-6
- 의존: FR-004

### FR-013
- 요구사항: 시스템은 일반 scene 전환 시 `TearingDown 화면 표시 -> OnExit 수행 -> Loading 화면 표시 -> OnEnter 수행 -> 대상 본 화면` 순서를 보장해야 한다.
- 세부 조건:
  - `TearingDown` 화면은 `OnExit` 수행 전에 최소 1프레임 이상 먼저 표시되어야 한다.
  - `OnExit`는 `TearingDown` phase 내부에서 수행되어야 한다.
  - `Loading` 화면은 `OnEnter` 수행 전에 최소 1프레임 이상 먼저 표시되어야 한다.
  - `OnEnter`는 `Loading` phase 내부에서 수행되어야 한다.
  - 두 전환 화면 사이에 대상 scene 본 화면이 먼저 노출되어서는 안 된다.
- 출처: IL-Q9, BR-6
- 의존: FR-003, FR-012, FR-017

### FR-014
- 요구사항: 시스템은 `Main Menu`의 `Quit` 입력 시 `Yes`와 `No` 버튼을 가진 popup을 표시해야 한다.
- 세부 조건:
  - `No`를 선택하면 popup을 닫고 `Main Menu` scene로 복귀해야 한다.
  - popup이 열린 동안 사용자가 종료 의사를 명시적으로 결정할 수 있어야 한다.
- 출처: IL-Q4, IL-Q10, BR-8
- 의존: FR-007

### FR-015
- 요구사항: 시스템은 `Quit` popup에서 `Yes`를 선택하면 scene 종료 흐름으로 간주하여 `TearingDown` 화면을 먼저 표시하고, phase 내부 정리 작업을 수행한 뒤 애플리케이션을 종료해야 한다.
- 세부 조건:
  - `Yes` 선택 후 즉시 프로세스를 종료해서는 안 된다.
  - 종료 전 `TearingDown` 텍스트와 전환 배경이 최소 1프레임 이상 먼저 표시되어야 한다.
  - 종료 관련 `OnExit`는 `TearingDown` phase 내부에서 수행되어야 한다.
- 출처: IL-Q10, BR-6, BR-8
- 의존: FR-014, FR-017

### FR-016
- 요구사항: 시스템은 `Title`, `Main Menu`, `Scenarios`, `Option`, `Play`, `End`의 본 scene 배경을 모두 회색으로 표시해야 한다.
- 세부 조건:
  - 배경색 적용은 scene별로 일관되어야 한다.
  - scene별 버튼과 텍스트는 회색 배경 위에서 식별 가능해야 한다.
- 출처: IL-Q6, BR-7
- 의존: FR-004

### FR-017
- 요구사항: 시스템은 전환 화면에서 `Loading` 또는 `TearingDown` 텍스트를 표시하고 배경을 옅은 녹색으로 표시해야 한다.
- 세부 조건:
  - `Loading`과 `TearingDown`은 현재 전환 단계에 맞는 텍스트만 노출해야 한다.
  - 전환 화면의 배경색은 본 scene 회색 배경과 시각적으로 구분되어야 한다.
- 출처: IL-Q6, IL-Q9, IL-Q10, BR-6, BR-7
- 의존: FR-013, FR-015

### FR-018
- 요구사항: 시스템은 ImGui를 scene 전환용 버튼과 텍스트 표시 용도로 사용해야 한다.
- 세부 조건:
  - scene UI는 버튼과 텍스트 중심의 단순한 구성이어야 한다.
  - docking, ini 저장 등 복잡한 editor 지향 UI 기능은 활성 요구사항에 포함하지 않아야 한다.
- 출처: IL-Q1, IL-Q7, BR-4, BR-9
- 의존: FR-001

### FR-019
- 요구사항: 시스템은 윈도우 크기와 해상도에 대해 기존 코드의 기본값을 유지해야 한다.
- 세부 조건:
  - 본 요구사항은 새 외부 설정 파일 도입 없이 기존 기본 실행값을 그대로 사용하는 것을 의미한다.
- 출처: IL-Q7, BR-7
- 의존: FR-001, NFR-003

## 5. 비기능 요구사항 (NFR)

### NFR-001
- 요구사항: 시스템은 기존 코드의 `CBase`, `App`, `AppCtx`, `CMake` 구조를 최대한 유지하면서 템플릿 목적에 맞게 정리해야 한다.
- 품질 의도:
  - 기존 코드 기반을 재작성보다 정리와 승격 중심으로 활용해야 한다.
- 출처: IL-DI, BR-1, BR-4
- 의존: 없음

### NFR-002
- 요구사항: 시스템은 scene 전이 흐름을 처음 보는 개발자도 코드만 읽고 추적할 수 있을 정도로 가독성 있게 구성해야 한다.
- 품질 의도:
  - scene 식별, 전이 요청, lifecycle 호출, 전환 상태가 분산되지 않고 이해 가능한 형태여야 한다.
- 출처: IL-Q8, BR-4
- 의존: FR-003, FR-012, FR-013

### NFR-003
- 요구사항: 시스템은 scene 전이 규칙과 scene 구성을 외부 yaml/json 등의 설정 파일에 의존하지 않고 내부 코드로 명시해야 한다.
- 품질 의도:
  - 런타임 해석용 설정 파일을 추가하지 않아야 한다.
  - 전이 규칙은 컴파일 타임 또는 명시적 코드 구조에서 확인 가능해야 한다.
- 출처: IL-Q8, BR-3, BR-4
- 의존: FR-003, FR-004

### NFR-004
- 요구사항: 시스템은 완성 게임이 아니라 SDL2 + EnTT + ImGui 기반 앱/게임 시작점으로 재사용 가능한 베이스 프레임워크 템플릿이어야 한다.
- 품질 의도:
  - 특정 게임 콘텐츠보다 재사용성과 확장 가능성을 우선해야 한다.
- 출처: IL-Q1, IL-DI, BR-1, BR-2
- 의존: FR-001, FR-002, FR-004

### NFR-005
- 요구사항: 시스템은 현재 버전에서 실제 완성 게임 콘텐츠를 포함하지 않아야 한다. 단, Play 씬은 ECS/ctx 사용법 예시(GameState, onUpdate)를 포함하는 예시 구현을 제공한다.
- 품질 의도:
  - 본 버전은 scene 흐름, lifecycle, ctx 활용 예제 제공에 집중해야 한다.
  - 템플릿 사용자가 ctx 구조를 참고하여 자신의 게임 로직으로 교체할 수 있어야 한다.
- 출처: BR-3, IL-DI
- 의존: FR-008, FR-010

### NFR-006
- 요구사항: 시스템은 프로젝트 전체가 빌드 가능한 상태를 유지해야 한다.
- 품질 의도:
  - 템플릿은 새로운 프로젝트 시작점으로 즉시 사용할 수 있어야 한다.
- 출처: IL 확인 사실 12-17행, BR-10
- 의존: FR-001, NFR-001

### NFR-007
- 요구사항: 시스템은 프로젝트 전체가 테스트 가능한 상태를 유지하고 정의된 테스트를 통과해야 한다.
- 품질 의도:
  - 템플릿 변경 시 기본 회귀 검증이 가능해야 한다.
- 출처: IL 확인 사실 12-17행, BR-10
- 의존: NFR-006

### NFR-008
- 요구사항: 시스템은 실행 검증이 가능한 상태를 유지해야 한다.
- 품질 의도:
  - 실제 실행 시 scene 전환, popup, 전환 화면 표시가 눈으로 확인 가능해야 한다.
- 출처: IL 확인 사실 12-17행, BR-10
- 의존: FR-006, FR-007, FR-008, FR-009, FR-010, FR-011, FR-013, FR-015

### NFR-009
- 요구사항: 시스템은 `requirements`, `architecture`, `test_strategy`, `README` 문서와 구현 결과가 일관되게 유지되도록 문서화되어야 한다.
- 품질 의도:
  - 템플릿 사용자가 구조, 빌드, 테스트, 실행 방법을 문서만으로 이해할 수 있어야 한다.
- 출처: BR-2, BR-10, IL 확인 사실 12-17행
- 의존: NFR-006, NFR-007, NFR-008

## 6. 요구사항 간 주요 의존 관계 요약
- scene 관리 기반: FR-001 → FR-002 → FR-003 → FR-012 → FR-013
- 초기 사용자 흐름: FR-004 → FR-005 → FR-006 → FR-007
- scene 이동 흐름: FR-007/FR-008/FR-009/FR-010/FR-011 → FR-013
- 종료 흐름: FR-007 → FR-014 → FR-015
- 화면 정책: FR-016, FR-017은 각 scene 및 전환 렌더링 요구사항의 공통 표현 정책이다.
- 구조 제약: NFR-001, NFR-002, NFR-003은 FR-002, FR-003, FR-004의 설계 방식 제약이다.
- 템플릿 완성도: NFR-006, NFR-007, NFR-008, NFR-009는 구현 완료 기준을 보완한다.

## 7. 출처 추적 매트릭스
- IL-Q1 -> FR-001, FR-002, FR-018, NFR-004
- IL-Q2 -> FR-004, FR-005
- IL-Q3 -> FR-005, FR-006
- IL-Q4 -> FR-007, FR-014
- IL-Q5 -> FR-008, FR-009, FR-010, FR-011
- IL-Q6 -> FR-016, FR-017
- IL-Q7 -> FR-018, FR-019
- IL-Q8 -> FR-002, FR-003, NFR-002, NFR-003
- IL-Q9 -> FR-012, FR-013, FR-017
- IL-Q10 -> FR-014, FR-015, FR-017
- IL-DI -> FR-002, FR-003, NFR-001, NFR-004, NFR-005
- BR-1 -> NFR-001, NFR-004
- BR-2 -> FR-002, FR-003, NFR-004, NFR-009
- BR-3 -> FR-004, FR-010, NFR-003, NFR-005
- BR-4 -> FR-001, FR-002, FR-003, FR-018, NFR-001, NFR-002, NFR-003
- BR-5 -> FR-004, FR-005, FR-006, FR-007, FR-008, FR-009, FR-010, FR-011
- BR-6 -> FR-012, FR-013, FR-015, FR-017
- BR-7 -> FR-016, FR-017, FR-019
- BR-8 -> FR-007, FR-014, FR-015
- BR-9 -> FR-018
- BR-10 -> NFR-006, NFR-007, NFR-008, NFR-009

## 8. 구현 시 우선순위 제안
1. FR-001 ~ FR-005, NFR-001 ~ NFR-003: 앱 골격, ctx/dispatcher, scene 등록 구조 확정
2. FR-006 ~ FR-011: 각 scene UI 및 기본 전이 구현
3. FR-012 ~ FR-017: lifecycle, 전환 화면, Quit popup, 공통 화면 정책 구현
4. FR-018 ~ FR-019, NFR-004 ~ NFR-009: 사용 범위 정리, 빌드/테스트/실행/문서 완결
