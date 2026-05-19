# app_base

app_base는 SDL2 + EnTT + ImGui 기반의 앱/게임 시작 프레임워크 템플릿이다. 완성 게임을 제공하는 저장소가 아니라, scene 전이 구조와 런타임 컨텍스트 관리, 기본 UI 흐름, 빌드/테스트 골격을 재사용할 수 있도록 정리한 출발점 프로젝트다.

현재 구현은 다음 목적에 맞춰 정리되어 있다.
- SDL2 윈도우/렌더러 기반 메인 루프
- EnTT `ctx` 기반 공유 런타임 컨텍스트 관리
- EnTT `dispatcher` 기반 scene 전이 요청 처리
- ImGui 기반 최소 버튼/텍스트 UI
- scene lifecycle(`OnEnter`, `OnExit`)와 전환 화면 예제
- 기본 빌드/테스트/실행 검증 루트

관련 상세 문서는 `docs/`와 `reports/` 아래에 정리되어 있다.

## 1. 프로젝트 개요

이 프로젝트는 새 SDL2 애플리케이션 또는 게임을 시작할 때 바로 복제해서 수정할 수 있는 템플릿이다. 현재 코드는 다음과 같은 예제 흐름을 실제로 포함한다.
- `Title`
- `Main Menu`
- `Scenarios`
- `Option`
- `Play`
- `End`

핵심은 “scene가 어떻게 들어오고 나가며, 중간 전환 화면과 종료 흐름을 어떻게 다루는가”를 코드 수준에서 바로 읽고 바꿀 수 있게 해 두었다는 점이다.

## 2. 현재 구현된 핵심 기능/구조

### 2.1 Scene lifecycle
각 본 scene는 `OnEnter`, `OnExit`, `OnRender`, `OnUpdate` 훅 형태로 구성된다.
- `OnEnter`, `OnExit`, `OnRender`: 필수 훅, 각 씬 파일(`libsrc/scenes/Scene_*.cpp`)에 구현
- `OnUpdate`: 선택적 훅, nullptr이면 스킵. 전이 중에도 스킵됨. 현재 Play 씬만 구현

일반 scene 전환 순서는 현재 다음과 같다.
1. `TearingDown` 화면을 먼저 1프레임 이상 표시
2. `TearingDown` phase 안에서 현재 scene `OnExit` 수행
3. 정리 완료 즉시 `Loading` 화면으로 진행
4. `Loading` 화면을 먼저 1프레임 이상 표시
5. `Loading` phase 안에서 대상 scene `OnEnter` 수행
6. 준비 완료 즉시 대상 scene 본 화면 표시

앱 최초 시작 시에는 별도 `Loading` 없이 `Title` scene으로 바로 진입한다.

### 2.2 Scene 구성
현재 UI 흐름은 아래와 같다.
- Title
  - `Title` 텍스트
  - `Start to Main Menu` 버튼
- Main Menu
  - `Start Scenario`
  - `Option`
  - `Quit`
- Scenarios
  - `Scenario 1` ~ `Scenario 4`
  - `Back To Main Menu`
- Option
  - `Back To Main Menu`
- Play
  - 상단 1/3: ImGui HUD (선택한 scenario 번호, 씬 점수, 전역 상태 표시)
  - 하단 2/3: SDL Renderer 게임 드로잉 (삼각형 회전/원 맥박/사각형 고정 애니메이션)
  - `Game Over` 버튼
- End
  - `To Main Menu`

현재 버전에서는 `Scenario 1~4`가 모두 `Play` scene으로 이동하며, 선택 번호는 `SceneTransitionRequest`의 `std::optional<PlayArgs>`로 전달되어 Play HUD에 표시된다.

### 2.3 Transition 화면과 배경 정책
현재 구현된 배경 정책은 다음과 같다.
- 본 scene 배경: 회색 계열 `(110, 110, 110)`
- transition 화면 배경: 옅은 녹색 계열 `(210, 235, 210)`

전환 화면에서는 `TearingDown` 또는 `Loading` 텍스트와 현재/대상 scene 정보, lifecycle 상태를 함께 표시한다.

### 2.4 Quit popup
`Main Menu`의 `Quit` 버튼을 누르면 ImGui modal popup이 열린다.
- `Yes`: `TearingDown` 화면 표시 후 phase 내부 정리 작업을 수행하고 즉시 앱 종료
- `No`: popup만 닫고 `Main Menu` 유지

### 2.5 EnTT ctx / dispatcher 사용 방식
공유 상태는 `entt::registry::ctx()`에 저장된다.
- `AppCtx`: SDL window, renderer, font, joystick, sound 등 런타임 자원
- `entt::dispatcher`: scene 전이/종료 요청 이벤트 버스
- `SceneRuntime`: 현재 scene, pending scene, phase, 종료 플래그, playArgs 등
- `GameState`: 씬 간 공유 누적 상태 (playerName, level, score, gold, playTimeSec)

`SceneDef.h`에 위 모든 공유 타입이 정의되어 있다. SDL/EnTT 의존성이 없어 어디서든 include 가능하다.

UI 버튼은 scene를 직접 바꾸지 않고 `dispatcher`에 전이 요청 이벤트를 넣고, 실제 상태 변경은 앱 루프에서 순차 처리한다.

### 2.6 ImGui 사용 범위
ImGui는 현재 최소한의 텍스트/버튼 UI 용도로만 사용한다.
- scene 제목/설명 텍스트
- 전환 버튼
- Quit modal popup

`IniFilename`과 `LogFilename`은 비활성화되어 있으며, docking이나 editor 지향 기능을 템플릿 기본값으로 제공하지 않는다.

## 3. 디렉터리 구조 요약

```text
.
├── CMakeLists.txt
├── cmake/
│   ├── 020_SDL2_support.cmake
│   ├── 050_entt_support.cmake
│   ├── 051_imgui.cmake
│   └── glog.cmake
├── docs/
│   ├── basic_requirements.md
│   ├── requirements.md
│   ├── architecture.md
│   ├── test_strategy.md
│   └── interview_log.md
├── include/
│   ├── App.h
│   ├── Log_Util.h
│   └── libmodule.h
├── 3rdparty/
│   ├── entt/
│   └── imgui/
├── libsrc/
│   ├── App.cpp
│   ├── CBase.cpp
│   ├── CBase.h
│   ├── SDL2_Ctx.cpp
│   ├── SDL2_Ctx.h
│   ├── SceneDef.h
│   ├── main_entry.cpp
│   ├── main_entry.h
│   ├── libmodule.cpp
│   └── scenes/
│       ├── SceneUtil.h
│       ├── SceneUtil.cpp
│       ├── SceneMap.h / SceneMap.cpp
│       ├── Scene_Title.h / Scene_Title.cpp
│       ├── Scene_MainMenu.h / Scene_MainMenu.cpp
│       ├── Scene_Scenarios.h / Scene_Scenarios.cpp
│       ├── Scene_Option.h / Scene_Option.cpp
│       ├── Scene_Play.h / Scene_Play.cpp
│       └── Scene_End.h / Scene_End.cpp
├── resource/
│   ├── fonts/
│   └── pics/
├── reports/
│   ├── audit_report.md
│   ├── build_report.md
│   ├── run_report.md
│   ├── test_report.md
│   ├── failure_summary.md
│   └── process_timeline.md
├── src/
│   └── main.cpp
└── test/
    ├── test_main.cpp
    ├── testcase_00.cpp
    ├── testcase_01_runtime.cpp
    └── testcase_02_gui_static.cpp
```

참고할 만한 파일 역할은 다음과 같다.
- `src/main.cpp`: glog 초기화 후 `main_entry()` 호출
- `libsrc/main_entry.cpp`: `App` 생성/시작/대기
- `libsrc/App.cpp`: dispatcher 연결, transition 상태 머신, ImGui 렌더링
- `libsrc/SceneDef.h`: 공유 타입 정의 (TransitionPhase, PlayArgs, GameState, SceneRuntime 등). scenes/SceneMap.h 를 include
- `libsrc/scenes/SceneUtil.h/.cpp`: 씬 공통 유틸 함수 (BeginFullscreenUi 등)
- `libsrc/scenes/SceneMap.h/.cpp`: SceneId, SceneHook, SceneDefinition, GetSceneMap(). 새 씬 추가 시 이 파일만 수정
- `libsrc/scenes/Scene_*.h/.cpp`: 씬별 OnEnter/OnExit/OnRender/OnUpdate 구현
- `libsrc/SDL2_Ctx.cpp`: SDL window/renderer/font/sound/joystick 초기화
- `libsrc/CBase.cpp`: 기본 루프 스레드 실행 엔진
- `test/`: gtest 기반 자동 테스트
- `docs/`: 요구사항/아키텍처/테스트 전략 문서
- `reports/`: 빌드/테스트/실행 검증 결과

## 4. 의존성 및 빌드 방법

## 4.1 주요 의존성
현재 CMake와 코드 기준으로 필요한 주요 의존성은 다음과 같다.
- CMake 3.11+
- C++17 컴파일러 (`g++`)
- pthread/Threads
- SDL2
- SDL2_ttf
- SDL2_image
- SDL2_mixer
- glog
- gtest

서드파티 포함 방식은 다음과 같다.
- EnTT: `FetchContent`로 가져오며 소스 위치는 `3rdparty/entt`
- ImGui: `FetchContent`로 가져오며 소스 위치는 `3rdparty/imgui`

즉, SDL2 계열 라이브러리, glog, gtest는 시스템에 준비되어 있어야 하고, EnTT/ImGui는 CMake 과정에서 현재 프로젝트에 포함된다.

Ubuntu 계열에서는 보통 아래 패키지들이 필요하다.
```bash
sudo apt install -y \
  build-essential cmake pkg-config \
  libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev \
  libgoogle-glog-dev libgtest-dev
```

참고:
- 테스트 링크는 현재 `gtest` 타깃을 사용한다.
- 빌드 리포트 기준 Debug 빌드는 정상 확인되었다.

## 4.2 빌드
저장소 루트(`/home/ubuntu/00_work/04_wc/tmpl`)에서 실행한다.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

현재 산출물 이름은 다음과 같다.
- 앱: `build/Debug/app_base-1.0.out`
- 테스트: `build/Debug/test_app_base-1.0.out`

기본 `CMAKE_BUILD_TYPE`를 지정하지 않으면 `Debug`가 사용된다.

## 5. 실행 방법

가장 직접적인 실행 방법은 다음 중 하나다.

방법 1: 저장소 루트에서 실행
```bash
./build/Debug/app_base-1.0.out
```

방법 2: 산출물 디렉터리로 이동 후 실행
```bash
cd build/Debug
./app_base-1.0.out
```

보조 스크립트도 포함되어 있다.
```bash
./r.sh
```

실행 시 현재 확인된 특성
- 창 제목은 `app_base`
- 기본 윈도우 크기는 `1024x768`
- 오디오 장치가 없는 환경에서는 SDL_mixer 경고 후 계속 진행할 수 있음
- 조이스틱이 없어도 경고만 출력하고 계속 진행함

## 6. 테스트 방법

자동 테스트는 gtest 기반으로 구성되어 있다.

### 6.1 테스트 목록 확인
```bash
./build/Debug/test_app_base-1.0.out --gtest_list_tests
```

### 6.2 전체 테스트 실행
```bash
./build/Debug/test_app_base-1.0.out
```

또는 보조 스크립트 사용:
```bash
./tt.sh
```

현재 자동 테스트 결과는 다음 범위에 집중한다.
- `FPS_Ctrl` 입력/시간 계산 보조 로직
- `CBase` 실행 순서
- loop procedure 필수 여부(death test 포함)
- 테스트 바이너리 링크 및 실행 가능 상태
- `libsrc/App.cpp`의 scene/transition 상수, dispatcher 연결, 최소 ImGui UI 구성에 대한 GUI 정적 검토

`./build/Debug/test_app_base-1.0.out --gtest_list_tests` 기준 현재 자동 테스트는 5개 test suite, 총 8개 테스트로 구성되어 있다.
이 중 `test/testcase_02_gui_static.cpp`에 GUI 정적 검토 테스트 3건이 추가되어, 화면 로직 관련 소스 수준 요구사항을 자동 확인한다.
다만 이는 GUI 시각 결과를 직접 검증하는 테스트가 아니라, scene/전환/버튼/quit popup 관련 구현 요소가 코드에 반영되어 있는지를 정적으로 점검하는 범위다.

## 7. headless/Docker 환경에서의 실행/검증 한계

이 프로젝트는 GUI 앱이므로 headless 또는 Docker 환경에서는 “실행 가능 여부 확인”과 “실제 화면 시각 검증”을 분리해서 봐야 한다.

### 7.1 가능한 것
- 앱이 시작 직후 즉시 크래시하지 않는지 확인
- 일정 시간 동안 프로세스가 살아 있는지 확인
- Xvfb 기반 가상 디스플레이에서 smoke run 수행
- 비GUI 자동 테스트 실행
- GUI 정적 검토 테스트를 통한 scene/전환/UI 구성 코드 점검

### 7.2 아직 자동으로 확정하지 않는 것
다음 항목은 현재 문서/리포트 기준으로 완전 자동 GUI 시각 검증이 끝났다고 보면 안 된다.
- 각 scene의 실제 화면이 사람 눈에 의도대로 보이는지
- `TearingDown` / `Loading` 텍스트 노출 체감
- 회색 / 옅은 녹색 배경 구분
- Quit popup의 실제 시각적 동작
- ImGui 버튼 클릭 기반 전체 E2E 흐름

즉, headless 환경의 smoke run은 “시작 가능성” 확인이고, “화면 품질/전이 체감”까지 확정하는 검증은 아니다.

### 7.3 smoke run 예시
현재 실행 리포트에서 사용한 예시는 다음과 같다.

```bash
xvfb-run -a timeout 5s ./build/Debug/app_base-1.0.out
```

리포트 기준 해석:
- 종료 코드 `124`는 `timeout 5s`에 의해 종료되었음을 의미
- 즉, 앱이 즉시 죽은 것이 아니라 최소 5초 동안 실행 상태를 유지한 smoke 성공으로 볼 수 있음

추가로 수동 확인이 가능하다면 실제 데스크톱 환경 또는 Xvfb + 입력 자동화 도구를 별도로 준비하는 편이 낫다.

## 8. 현재 템플릿을 확장할 때 먼저 수정할 곳

새 프로젝트로 확장할 때는 보통 아래 순서로 보는 것이 가장 빠르다.

### 8.1 새 씬 추가 절차 — `Scenario2_Play` 튜토리얼

Scenarios 씬에서 Scenario 2를 눌렀을 때 전이하는 독립 씬 `Scene::Scenario2Play`를 추가하는 전체 과정을 단계별로 설명한다. Play 씬과 동일하게 Game Over 버튼을 누르면 End로 전이한다.

#### 8.1.1 SceneId enum에 ID 추가

`libsrc/SceneDef.h`의 `SceneId` enum에 새 항목을 추가한다.

```cpp
// libsrc/SceneDef.h
enum class SceneId {
  Title,
  MainMenu,
  Scenarios,
  Option,
  Play,
  Scenario2Play,   // ← 추가
  End,
};
```

`ToString()` 함수도 switch에 케이스를 추가해야 컴파일 경고가 발생하지 않는다.

```cpp
// libsrc/SceneDef.h - ToString(SceneId) switch
case SceneId::Scenario2Play: return "Scenario 2 Play";
```

#### 8.1.2 씬 파일 생성

파일명은 `Scene_Scenario2Play.h / .cpp`로 생성한다. 네임스페이스는 `Scene::Scenario2Play`로 선언한다.

**Scene_Scenario2Play.h**

```cpp
#pragma once
#include "entt/entt.hpp"

namespace Scene::Scenario2Play {
  void OnEnter(entt::registry &ECS);
  void OnExit(entt::registry &ECS);
  void OnRender(entt::registry &ECS);
}
```

**Scene_Scenario2Play.cpp**

```cpp
#include "Scene_Scenario2Play.h"
#include "SceneUtil.h"
#include "SceneDef.h"
#include "imgui.h"

namespace Scene::Scenario2Play {

void OnEnter(entt::registry &ECS) {
  OnGenericSceneEnter(ECS, SceneId::Scenario2Play);
}

void OnExit(entt::registry &ECS) {
  OnGenericSceneExit(ECS, SceneId::Scenario2Play);
}

void OnRender(entt::registry &ECS) {
  RenderSceneChrome(ECS, SceneId::Scenario2Play, "Scenario 2 Play",
                   "Scenario 2 전용 플레이 씬 예시.");

  auto &dispatcher = ECS.ctx().get<entt::dispatcher>();
  if (ImGui::Button("Game Over", ImVec2(220.0f, 40.0f))) {
    dispatcher.enqueue<SceneTransitionRequest>(
        SceneTransitionRequest{SceneId::End});
  }

  EndFullscreenUi();
}

}  // namespace Scene::Scenario2Play
```

`OnGenericSceneEnter`, `OnGenericSceneExit`, `RenderSceneChrome`, `EndFullscreenUi`는 `SceneUtil.h`에 정의된 공통 유틸이다. `SceneDef.h`의 `SceneId::Scenario2Play`와 `SceneTransitionRequest`를 그대로 사용한다.

#### 8.1.3 SceneMap.cpp에 등록

`libsrc/scenes/SceneMap.cpp` 상단에 헤더를 추가하고, `GetSceneMap()` 반환 맵에 항목을 등록한다.

**include 추가**

```cpp
// libsrc/scenes/SceneMap.cpp 상단 include 블록
#include "Scene_Scenario2Play.h"   // ← 추가
```

**GetSceneMap() 등록**

현재 코드 구조:

```cpp
std::map<SceneId, SceneDefinition> GetSceneMap() {
  return {
      {SceneId::Title,
       {Scene::Title::OnEnter, Scene::Title::OnExit, Scene::Title::OnRender, nullptr}},
      // ... 생략 ...
      {SceneId::End,
       {Scene::End::OnEnter, Scene::End::OnExit, Scene::End::OnRender, nullptr}},
  };
}
```

`End` 항목 뒤에 다음을 추가한다:

```cpp
      {SceneId::Scenario2Play,
       {Scene::Scenario2Play::OnEnter,
        Scene::Scenario2Play::OnExit,
        Scene::Scenario2Play::OnRender,
        nullptr}},   // onUpdate 없으면 nullptr
```

`CMakeLists.txt`는 `libsrc/scenes/*.cpp` glob을 사용하므로 **별도 수정이 필요 없다**.

#### 8.1.4 Scenarios 씬에서 전이 연결

`libsrc/scenes/Scene_Scenarios.cpp`의 `OnRender`에서 Scenario 2 버튼 클릭 시 `Scenario2Play`로 전이하도록 분기를 추가한다.

현재 코드:

```cpp
// Scene_Scenarios.cpp - OnRender
for (int scenario = 1; scenario <= 4; ++scenario) {
  const std::string label = "Scenario " + std::to_string(scenario);
  if (ImGui::Button(label.c_str(), ImVec2(220.0f, 40.0f))) {
    dispatcher.enqueue<SceneTransitionRequest>(
        SceneTransitionRequest{SceneId::Play, PlayArgs{scenario}});
  }
}
```

수정 후:

```cpp
for (int scenario = 1; scenario <= 4; ++scenario) {
  const std::string label = "Scenario " + std::to_string(scenario);
  if (ImGui::Button(label.c_str(), ImVec2(220.0f, 40.0f))) {
    if (scenario == 2) {
      // Scenario 2는 별도 씬으로 전이
      dispatcher.enqueue<SceneTransitionRequest>(
          SceneTransitionRequest{SceneId::Scenario2Play});
    } else {
      dispatcher.enqueue<SceneTransitionRequest>(
          SceneTransitionRequest{SceneId::Play, PlayArgs{scenario}});
    }
  }
}
```

#### 8.1.5 빌드 확인

```bash
bash bd.sh
```

`CMakeLists.txt`의 glob이 `Scene_Scenario2Play.cpp`를 자동으로 포함하므로 별도 cmake 수정 없이 빌드가 성공해야 한다.

---

위 5단계가 새 씬을 추가하는 전체 절차이다. 요약하면:

| 단계 | 파일 | 작업 |
|------|------|------|
| 8.1.1 | `SceneDef.h` | SceneId enum + ToString() 추가 |
| 8.1.2 | `scenes/Scene_Xxx.h/.cpp` | 네임스페이스 + OnEnter/OnExit/OnRender 구현 |
| 8.1.3 | `scenes/SceneMap.cpp` | include + GetSceneMap 등록 |
| 8.1.4 | 전이 출발 씬의 `.cpp` | 버튼 → dispatcher.enqueue 연결 |
| 8.1.5 | — | `bash bd.sh` 빌드 확인 |

### 8.2 ctx 사용 패턴

```cpp
// emplace (App 시작 시 1회)
ECS.ctx().emplace<GameState>();

// 읽기
const auto &gs = ECS.ctx().get<GameState>();

// 수정
ECS.ctx().get<GameState>().gold += 100;

```

### 8.3 `libsrc/SDL2_Ctx.h`, `libsrc/SDL2_Ctx.cpp`
런타임 자원과 창 기본값을 바꾸고 싶을 때 본다.
- 윈도우 크기/위치
- SDL 초기화 범위
- 폰트 로딩
- 사운드/조이스틱 초기화

### 8.4 `src/main.cpp`, `libsrc/main_entry.cpp`, `include/App.h`
앱 진입 구조나 상위 인터페이스를 바꿀 때 본다. 현재는 매우 얇은 래퍼 역할이라 scene 수정만 한다면 자주 건드릴 필요는 없다.

### 8.5 `test/`
템플릿을 확장하면서 최소 회귀 테스트를 같이 늘리는 것을 권장한다.
- 순수 로직은 가능하면 gtest로 분리
- GUI 클릭/시각 요소는 smoke 또는 수동 검증과 분리

### 8.6 문서
구조를 실제로 바꿨다면 아래 문서도 같이 맞추는 편이 좋다.
- `docs/requirements.md`
- `docs/architecture.md`
- `docs/test_strategy.md`

## 9. 문서 및 검증 자료

프로젝트 상태를 더 자세히 보려면 아래 문서를 참고하면 된다.
- `docs/basic_requirements.md`: 기본 요구 범위
- `docs/requirements.md`: 구현 요구사항 상세
- `docs/architecture.md`: 구조 설계와 scene/runtime 설계
- `docs/test_strategy.md`: 자동화/수동 검증 경계와 테스트 전략
- `reports/build_report.md`: 빌드 검증 결과
- `reports/test_report.md`: 테스트 실행 결과
- `reports/run_report.md`: Xvfb 기반 실행 smoke 결과
- `reports/audit_report.md`: 감사 결과 및 이슈 정리

## 10. 현재 상태 요약

현재 이 저장소는 다음 용도로 바로 사용할 수 있다.
- SDL2 앱/게임 시작 템플릿
- scene 전이 구조 예제 (SceneDefinition + 함수 포인터 등록 방식)
- EnTT `ctx`/`dispatcher` 사용 예제 (AppCtx, SceneRuntime, GameState)
- 씬 간 데이터 전달 예제 (`SceneTransitionRequest` + `std::optional<PlayArgs>`)
- Play 씬 ECS/GameState 예시 구현 (onUpdate, entity/component 재생성, GameState 누적)
- ImGui 최소 UI + SDL Renderer 게임 드로잉 분리 예제
- CMake + gtest 기반 기본 프로젝트 골격

반면, 다음은 아직 템플릿 사용자가 직접 채워 넣어야 하는 영역이다.
- 실제 게임 규칙/콘텐츠
- scenario별 개별 플레이 로직
- 자동 GUI 시각 검증 체계
- 고급 editor 기능 또는 데이터 기반 scene 설정
