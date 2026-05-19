# 빌드 검증 리포트

## 1. 검증 개요
- 프로젝트 경로: `/home/ubuntu/00_work/04_wc/tmpl`
- 공식 프로젝트명: `app_base`
- 검증 시각: 2026-05-14T00:20:42+00:00
- 검증 환경:
  - OS: Linux wstation-HP-ZBook 6.17.0-20-generic #20~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Mar 19 01:28:37 UTC 2 x86_64 x86_64 x86_64 GNU/Linux
  - 작업 디렉터리: `/home/ubuntu/00_work/04_wc/tmpl`
  - CMake: `/usr/bin/cmake`
  - CTest: `/usr/bin/ctest`

## 2. 실행 명령
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
```

## 3. 실행 결과
### 3.1 Configure
- 결과: 성공
- 주요 출력:
  - `-- Configuring done (0.2s)`
  - `-- Generating done (0.0s)`
  - `-- Build files have been written to: /home/ubuntu/00_work/04_wc/tmpl/build`

### 3.2 Build
- 결과: 성공
- 주요 출력:
  - `ninja: no work to do.`
- 해석:
  - 기존 Debug 빌드 산출물이 최신 상태이며, 추가 컴파일은 발생하지 않았다.

## 4. 경고 및 특이사항
- Configure 단계에서 CMake 개발자 경고 1건 확인:
  - `FetchContent.cmake` 관련 `DOWNLOAD_EXTRACT_TIMESTAMP` / `CMP0135` 경고
  - 호출 위치: `cmake/051_imgui.cmake:4`, `CMakeLists.txt:25`
- 영향도:
  - 이번 검증에서는 빌드 실패를 유발하지 않았음
  - 다만 외부 아카이브 재다운로드 시 타임스탬프 기반 재빌드 판단에 영향을 줄 수 있음

## 5. 산출물 확인
- 앱 실행 파일 존재: `build/Debug/app_base-1.0.out`
- 테스트 실행 파일 존재: `build/Debug/test_app_base-1.0.out`

## 6. 결론
- 빌드 검증 결과: 성공
- 프로젝트는 지정된 명령으로 Debug 구성 및 빌드 가능한 상태임
