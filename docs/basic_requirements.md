# Basic Requirements

## 1. 프로젝트 개요
- 프로젝트명: app_base
- 작업 경로: /home/ubuntu/00_work/04_wc/tmpl
- 프로젝트 성격: SDL2 기반 앱/게임의 시작점이 되는 공통 프레임워크 템플릿
- 개발 방식: 기존 코드를 거의 유지한 채 새 프로젝트 수준으로 정리 및 보강

## 2. 목표
- 기존 tmpl 코드를 기반으로 재사용 가능한 SDL2 앱/게임 베이스 프레임워크로 정리한다.
- scene 기반 상태 전이 구조를 예제 수준이 아닌 실제 시작점 수준으로 제공한다.
- SDL runtime context 를 EnTT ctx 로 관리하고, scene transition 은 EnTT dispatcher 를 활용하는 구조를 제공한다.
- 문서(requirements, architecture, test strategy, README)와 코드, 테스트, 실행 검증까지 포함한 완성된 시작 템플릿을 만든다.

## 3. 범위
### 포함
- SDL2 + EnTT + ImGui 기반 애플리케이션 구조
- scene/state 전이 예제 구현
- Title / Main Menu / Scenarios / Option / Play / End 씬 구현
- scene enter / exit handler 구현
- transition state (`TearingDown`, `Loading`) 구현
- Quit popup 구현
- 빌드/테스트/실행 검증
- 문서화

### 제외
- 실제 게임 플레이 로직
- scenario 별 개별 게임 내용 차별화
- 외부 yaml/json 기반 scene 설정
- docking, ini 저장 등 editor 지향 imgui 기능

## 4. 기술/구조 요구
- SDL2 기반으로 윈도우를 생성한다.
- SDL 관련 runtime context 는 EnTT ctx 에 저장한다.
- ImGui 를 포함하여 씬 전환 버튼 UI 를 구현한다.
- 기존 코드의 CBase / App / AppCtx / CMake 구조를 최대한 유지한다.
- scene transition 은 내부 코드로 명시적으로 작성한다.
- 외부 yaml 등의 설정 파일은 만들지 않는다.
- 구조를 처음 보는 사람도 scene 전이 흐름을 이해할 수 있어야 한다.
- EnTT ctx 와 dispatcher 를 scene 상태 관리와 전이 요청에 활용한다.

## 5. 씬 구성 및 동작
### Title
- `Title` 텍스트를 출력한다.
- `Start to Main Menu` 버튼을 제공한다.
- 버튼 클릭 시 Main Menu 로 이동한다.

### Main Menu
- `Start Scenario`, `Option`, `Quit` 버튼을 제공한다.
- `Start Scenario` 클릭 시 Scenarios 로 이동한다.
- `Option` 클릭 시 Option 으로 이동한다.
- `Quit` 클릭 시 종료 popup 을 띄운다.

### Scenarios
- `Scenario 1`, `Scenario 2`, `Scenario 3`, `Scenario 4` 버튼을 제공한다.
- `Back To Main Menu` 버튼을 제공한다.
- 현재 단계에서는 Scenario 1~4 모두 Play 씬으로 이동한다.
- Back 버튼은 Main Menu 로 이동한다.

### Option
- `Back To Main Menu` 버튼 하나만 제공한다.
- 버튼 클릭 시 Main Menu 로 이동한다.

### Play
- placeholder 씬으로 구현한다.
- `Game Over` 버튼 하나만 제공한다.
- 버튼 클릭 시 End 씬으로 이동한다.

### End
- `To Main Menu` 버튼 하나를 제공한다.
- 버튼 클릭 시 Main Menu 로 이동한다.

## 6. Scene Lifecycle 요구
- 각 씬은 진입 시점 handler 와 탈출 시점 handler 를 가진다.
- 일반 씬 전환 시 다음 순서를 따른다.
  1. 현재 씬 OnExit
  2. `TearingDown` 화면 2초 표시
  3. 대상 씬 OnEnter
  4. `Loading` 화면 2초 표시
  5. 대상 씬 본 화면 진입
- Quit 확정 시에도 씬 종료 흐름으로 간주하고 `TearingDown` 2초를 표시한다.

## 7. 화면 정책
- 본 씬의 배경은 모두 회색으로 한다.
- transition 화면(`Loading`, `TearingDown`) 배경은 옅은 녹색으로 한다.
- 윈도우 크기/해상도는 기존 코드 기본값을 유지한다.

## 8. Quit 정책
- Quit 클릭 시 popup 을 띄운다.
- popup 버튼은 `Yes`, `No` 이다.
- `No` 를 누르면 현재 씬(Main Menu)으로 복귀한다.
- `Yes` 를 누르면 종료 흐름으로 진입한다.

## 9. ImGui 사용 정책
- ImGui 는 씬 전환용 버튼 UI 와 텍스트 표시 용도로 사용한다.
- docking, ini 저장 등 복잡한 editor 기능은 포함하지 않는다.

## 10. 완료 기준
- 프로젝트 이름과 구조가 `app_base` 목적에 맞게 정리되어야 한다.
- 빌드가 성공해야 한다.
- 테스트가 성공해야 한다.
- 실행이 확인되어야 한다.
- requirements / architecture / test_strategy / README 문서가 정리되어야 한다.
