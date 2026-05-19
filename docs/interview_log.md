# Interview Log

- Project: app_base
- Working Path: /home/ubuntu/00_work/04_wc/tmpl
- Interview Start: 2026-05-13T12:00:59+00:00

## Confirmed Facts

- 프로젝트명은 `app_base` 이다.
- 작업 경로는 `/home/ubuntu/00_work/04_wc/tmpl` 이다.
- 기존 코드를 거의 유지한 채 새 프로젝트로 정리한다.
- 완료 기준은 다음을 모두 포함한다.
  - 이름/구조 정리
  - 빌드 통과
  - 테스트 통과
  - 실행 확인
  - 문서 정리

## Q&A Summary

### Q1. 프로젝트의 성격은 무엇인가?
A.
- SDL2 기반 앱/게임의 시작점이 되는 공통 프레임워크에 가깝다.
- 처음 개발 시작점이 되는 템플릿 정도로 보면 된다.
- SDL로 윈도우가 시작되고 그 컨텍스트들은 EnTT로 관리된다.
- cmake 디렉토리에 보이듯 imgui를 포함한다.

### Q2. 어떤 씬 구조를 가지는가?
A.
- 이 애플리케이션은 복수개의 씬을 가진다.
- 상태는 다음과 같다.
  - Title
  - Main Menu
  - Scenarios
  - Option
  - Play
  - End
- 이 state들은 각 씬에서 구현된 imgui의 버튼을 눌러서 전이된다.

### Q3. Title 씬은 어떻게 동작하는가?
A.
- Title 텍스트가 출력된다.
- 하단에는 `Start to Main Menu` 버튼이 있다.
- 버튼을 누르면 `Main Menu` 씬으로 이동한다.

### Q4. Main Menu 씬은 어떻게 동작하는가?
A.
- Main Menu에는 다음 3개의 버튼이 있다.
  - `Start Scenario`
  - `Option`
  - `Quit`
- `Start Scenario` 버튼을 누르면 `Scenarios` 씬으로 이동한다.
- `Option` 버튼을 누르면 `Option` 씬으로 이동한다.
- `Quit`는 애플리케이션 종료 흐름으로 진입한다.

### Q5. Scenarios / Play / End / Option 씬은 어떻게 동작하는가?
A.
- `Scenarios` 씬에는 다음 버튼이 있다.
  - `Scenario 1`
  - `Scenario 2`
  - `Scenario 3`
  - `Scenario 4`
  - `Back To Main Menu`
- Scenario 버튼 4개는 현재 모두 `Play` 씬으로 이동한다.
- `Back To Main Menu` 버튼은 `Main Menu` 씬으로 이동한다.
- `Play` 씬은 placeholder 씬이며 `Game Over` 버튼 하나만 가진다.
- `Game Over` 버튼을 누르면 `End` 씬으로 이동한다.
- `End` 씬에는 `To Main Menu` 버튼 하나가 있고 `Main Menu` 씬으로 이동한다.
- `Option` 씬에는 `Back To Main Menu` 버튼 하나만 있다.

### Q6. 공통 화면 정책은 무엇인가?
A.
- 각 본 씬의 바탕화면은 모두 회색이다.
- `Loading` / `TearingDown` 전환 화면 배경은 옅은 녹색이다.

### Q7. imgui 사용 범위는 어디까지인가?
A.
- 씬 전환용 버튼 UI만 있으면 된다.
- docking, ini 저장 등 복잡한 imgui 기본 세팅은 필요 없다.
- 윈도우 크기/해상도는 기존 코드 기본값을 유지한다.

### Q8. 씬 전이 설계 원칙은 무엇인가?
A.
- 씬 전이는 가독성 있게 설계되어야 한다.
- 외부 yaml 등의 파일까지 만들 필요는 없다.
- 내부 코드로 작성하되 이해하기 편리해야 한다.
- EnTT의 `dispatcher`와 `ctx`를 잘 활용하여 구현해야 한다.

### Q9. 씬 lifecycle 요구사항은 무엇인가?
A.
- 각 씬으로 이동할 때 `Loading`, `TearingDown` 텍스트가 표시되어야 한다.
- 각 씬에는 진입 시점 handler 와 탈출 시점 handler 가 구현되어야 한다.
- 일반 씬 전환 시 `TearingDown` 2초, `Loading` 2초를 보여준 뒤 본 화면으로 진입한다.

### Q10. Quit 흐름은 어떻게 동작하는가?
A.
- `Quit`는 popup을 띄운다.
- popup 버튼은 `Yes / No` 이다.
- `Yes`를 선택하면 종료 흐름으로 진행한다.
- 이때도 씬 변경 흐름으로 간주하며 `TearingDown` 표시가 포함되어야 한다.

## Design Intent Interpreted

- 이 프로젝트는 완성 게임이 아니라 SDL2 + EnTT + ImGui 기반 앱/게임을 시작할 때 재사용할 수 있는 공통 베이스 프레임워크이다.
- 기존 `/home/ubuntu/00_work/04_wc/tmpl` 코드를 최대한 유지하면서 구조, 이름, 문서를 정리해 템플릿 수준으로 승격하는 것이 목적이다.
- scene/state 전이의 최소 동작 예제와 lifecycle 예제를 제공해야 한다.
- App runtime context 는 EnTT ctx 로 관리하고, scene transition event 는 EnTT dispatcher 를 활용하는 방향이 의도에 부합한다.
