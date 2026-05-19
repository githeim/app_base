# 실행 검증 리포트

## 1. 검증 목적
현재 저장소 상태에서 공식 앱 바이너리 `app_base-1.0.out` 이 headless 환경에서도 실제로 기동되는지 확인하고, F-003 GUI 요구사항에 대해 실행 기반으로 확인 가능한 범위와 불가능한 범위를 명확히 분리한다.

## 2. 실행 환경
- 검증 시각: `2026-05-14T00:25:48+00:00`
- 프로젝트 경로: `/home/ubuntu/00_work/04_wc/tmpl`
- 공식 프로젝트명: `app_base`
- 앱 바이너리: `./build/Debug/app_base-1.0.out`
- 실행 방식: `xvfb-run` 기반 가상 디스플레이 smoke run
- 사용 도구:
  - `xvfb-run`: `/usr/bin/xvfb-run`
  - `timeout`: `/usr/bin/timeout`

## 3. 실행 명령
```bash
xvfb-run -a timeout 5s ./build/Debug/app_base-1.0.out
```

## 4. 실행 결과
- 명령 실행: 성공
- 관찰된 종료 코드: `124`
- 해석:
  - `timeout 5s` 에 의해 의도적으로 5초 후 종료되었다.
  - 즉시 크래시, SDL 초기화 실패, 프로세스 시작 실패는 이번 재실행에서 관찰되지 않았다.
  - 따라서 본 결과는 “앱이 headless 환경에서 최소한 기동되고 유지됨”을 확인한 smoke run 성공으로 해석할 수 있다.

## 5. 표준 출력/오류 주요 내용
확인된 로그:
- `ALSA lib pcm_dmix.c:1000:(snd_pcm_dmix_open) unable to open slave`
- `[Init_Joystick][192] :x: Warning: No joysticks connected!`
- `[Init_Sound][253] :x: SDL_mixer , Init Err`
- `[Init_SDL_ctx][61] :x: Warning: Sound init failed, continue without audio`
- `[DeInit_Sound][292] :x: DeInit Sound Done`
- `[DeInit_Joystick][236] :x: DeInit Joystick Done`
- `RUN_EXIT_CODE=124`

## 6. F-003 GUI 요구사항 검증 매트릭스
| 요구사항 | 코드상 확인 | 자동 테스트 확인 | 런타임 smoke 확인 | 현재 판정 |
|---|---|---|---|---|
| Scene: Title / Main Menu / Scenarios / Option / Play / End | 확인됨 (`libsrc/App.cpp` scene enum + scene map) | 확인됨 (`GuiStaticRequirementTest.DeclaresAllRequiredScenesAndTransitionPhases`) | 직접 전이 미확인 | 코드상 구현 확인, 시각 검증 미완료 |
| 일반 scene 배경: 회색 | 확인됨 (`kSceneBackground{110,110,110}` + phase none 시 적용) | 확인됨 (`GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`) | 색상 체감 미확인 | 코드상 확인, 시각 검증 미완료 |
| 전환 화면 배경: 옅은 녹색 | 확인됨 (`kTransitionBackground{210,235,210}`) | 확인됨 (`GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`) | 색상 체감 미확인 | 코드상 확인, 시각 검증 미완료 |
| 전환 텍스트: `Finishing Up` 2초 -> `Preparing` 2초 -> 대상 scene | 확인됨 (`TRANSITION_DURATION_MS=2000`, phase state machine, `ToString`) | 확인됨 (`GuiStaticRequirementTest.DeclaresAllRequiredScenesAndTransitionPhases`) | 텍스트 노출/체감 미확인 | 코드상 확인, 시각 검증 미완료 |
| Quit는 즉시 종료가 아니라 Yes / No popup | 확인됨 (`BeginPopupModal`, `Yes`, `No`) | 확인됨 (`GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`) | 실제 popup 표시 미확인 | 코드상 확인, 시각 검증 미완료 |
| Quit 확정 시에도 `Finishing Up` 흐름 적용 | 확인됨 (`StartQuit`, `exitAfterFinishing = true`) | 확인됨 (`GuiStaticRequirementTest.EncodesBackgroundPolicyAndQuitPopupFlow`) | 실제 확인 미확인 | 코드상 확인, 시각 검증 미완료 |
| 각 scene `OnEnter`, `OnExit` lifecycle 필요 | 확인됨 (scene map에서 enter/exit hook 연결, count/note 기록) | 확인됨 (`GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi`) | hook 실제 체감 미확인 | 코드상 확인, 시각 검증 미완료 |
| EnTT ctx/dispatcher 활용 | 확인됨 (`ctx().emplace`, `dispatcher.sink`, `enqueue/update`) | 확인됨 (`GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi`) | 내부 동작은 직접 관찰 불가 | 코드상/정적 테스트상 확인 |
| ImGui 최소 UI | 확인됨 (각 scene 버튼/텍스트 정의) | 확인됨 (`GuiStaticRequirementTest.EncodesLifecycleDispatcherCtxAndMinimumImGuiUi`) | 실제 렌더링 품질 미확인 | 코드상 확인, 시각 검증 미완료 |

## 7. 이번 실행으로 확인된 범위
- 공식 앱 바이너리 경로가 현재 문서와 일치한다.
- `app_base-1.0.out` 가 실행 가능 상태다.
- Xvfb 기반 가상 디스플레이에서 프로세스가 즉시 비정상 종료하지 않는다.
- 오디오 초기화 경고가 있어도 실행은 계속 진행된다.
- 최소한 초기 scene 렌더 루프까지 진입했을 가능성을 뒷받침하는 실행 지속성이 있다.

## 8. 이번 실행이 의미하는 수준
- 이번 검증은 GUI 기능 전체 완료를 입증하는 최종 시각 검증이 아니다.
- 정확한 표현은 “headless 환경에서의 실행 가능성 확인 + 코드/자동 테스트 근거 보강”이다.

## 9. 이번 실행과 보강 테스트로도 확인하지 못한 범위
- 실제 사람이 화면을 보며 Title / Main Menu / Scenarios / Option / Play / End 전이를 클릭 확인한 기록
- 각 scene의 텍스트와 버튼 배치가 요구사항과 동일한지에 대한 시각적 증빙
- `Preparing` / `Finishing Up` 문구가 실제로 약 2초 동안 노출되는지에 대한 시각 검증
- 회색 / 옅은 녹색 배경 구분의 시각 확인
- Quit popup 의 Yes / No 동작을 실제 입력으로 검증한 기록
- 실제 디스플레이 환경에서 폰트/레이아웃/포커스가 자연스러운지 검증한 기록

## 10. 환경 경고 및 제약
### 10.1 오디오
- ALSA / SDL_mixer 초기화 경고가 발생했다.
- 현재 실행 환경에 오디오 장치가 없거나 접근이 제한된 것으로 보인다.
- 애플리케이션은 경고 후 계속 진행했으므로 이번 smoke run의 실패로 분류하지 않는다.

### 10.2 입력 장치
- 조이스틱 미연결 경고가 발생했다.
- 템플릿 기동 자체에는 영향이 없었다.

### 10.3 GUI 시각 검증 제약
- 본 검증은 Xvfb 기반이므로 사람의 직접 관찰 기록이 아니다.
- 따라서 GUI 내용 자체는 “실행되었을 가능성이 높다” 수준까지만 말할 수 있으며, “시각적으로 요구사항을 모두 만족한다”로 확정할 수는 없다.

## 11. 관련 리포트와의 정합성
- `reports/build_report.md`: 공식 산출물명이 `build/Debug/app_base-1.0.out`, `build/Debug/test_app_base-1.0.out` 로 정리되어 있으며 본 문서와 일치한다.
- `reports/test_report.md`: 자동 테스트가 8건으로 확대되었고, GUI 요구사항의 코드상 근거를 일부 자동 검증한다는 점을 반영했다.
- `reports/failure_summary.md`: 실패 없음과 별도로 GUI 시각 검증 미완료를 분리 기재하도록 정합화했다.

## 12. 결론
- 현재 환경 기준 실행 검증 결과: smoke run 성공
- F-003 관점의 현재 상태:
  - 코드상 근거: 보강됨
  - 자동 테스트 근거: 보강됨
  - 실제 시각 검증: 여전히 미완료
- 종합 해석:
  - 앱 바이너리는 현재 저장소 상태에서 headless + Xvfb 환경에서 정상적으로 시작된다.
  - GUI 요구사항 중 여러 항목은 코드와 신규 정적 검토형 테스트로 “구현 존재”를 뒷받침할 수 있다.
  - 그러나 이번 증빙만으로 GUI 요구사항 전체가 시각적으로 완료되었다고 확정할 수는 없다.
  - 따라서 실행 관점의 가장 정확한 상태는 “기동 성공, 코드상/자동 테스트상 근거 보강, 시각 검증은 후속 수동 확인 필요”이다.
