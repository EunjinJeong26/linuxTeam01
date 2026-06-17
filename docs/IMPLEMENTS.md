# IMPLEMENTS.md — 구현 개선 항목 관리

> 데모 테스트 및 코드 리뷰 과정에서 발견된 버그·미구현 항목을 기록합니다.
> 결정이 확정되면 해당 항목에 선택안과 구현 완료 여부를 표시합니다.

---

## 항목 상태 표기

* 🔴 **미결정** — 선택안 논의 중
* 🟡 **결정됨** — 구현 대기
* ✅ **완료** — 구현 및 테스트 완료

---

## 현재 구현 현황 (2026-05-24 기준)

### 구현 완료

| 파일 | 담당 기능 |
| --- | --- |
| `client/src/db.c` | SQLite 초기화, 5개 테이블 스키마 생성 (`users`, `git_dirs`, `teams`, `members`, `logs`) |
| `client/src/session.c` | 세션 저장 / 로드 / 클리어 (`~/.devlog/session`) |
| `client/src/auth.c` | `register` / `login` / `logout` / `whoami` (로컬 DB 기반) |
| `client/src/team.c` | `team create` / `join` / `info` / `leave` |
| `client/src/sysinfo.c` | `/proc/stat`, `/proc/meminfo` 파싱으로 CPU/MEM 순간 스냅샷 수집 |
| `client/src/git_scan.c` | `git add` / `remove` / `list`, `.git` 재귀 탐색 및 최근 커밋 추출 |
| `client/src/log_write.c` | `post` — sysinfo + git + 텍스트 수집 후 DB 저장, UTF-8 150자 제한 |
| `client/src/log_read.c` | `show` — `--my`, `--date` 플래그 포함 팀 전체 조회 |

### 미구현

| 파일 | 담당 기능 | 비고 |
| --- | --- | --- |
| `client/src/notify.c` | `daemon status`, `notify on/off` | 함수 껍데기만 존재, UDS 통신 로직 없음 |
| `daemon/devlogd.c` | 정각 알림 발송, UDS 서버 소켓, TTY 감지 후 BEL 출력 | `TODO` 주석만 존재 |
| `client/src/api.c` | Spring Boot 서버 REST API 통신 (libcurl + JWT) | Phase 2 범위, 미착수 |

### 웹 프론트엔드 구현 완료 (2026-06-12, M1~M5)

| 위치 | 담당 기능 |
| --- | --- |
| `web/src/pages/` | 타임라인 그리드(`/`), 내 로그(`/me`), 팀 정보(`/team`), 로그인/회원가입 |
| `web/src/api/` | axios 클라이언트(JWT 인터셉터, 401 처리), raw↔내부 모델 매퍼, mock 분기(`VITE_USE_MOCK`) |
| `web/src/hooks/` | `useTeam`(팀 미소속 404 분기), `useTeamLogs`/`useMyLogs`(오늘만 60초 폴링) |
| `web/scripts/seed-dev.ps1` | Windows 검증용 시드 (계정·팀·로그 REST 생성) |

상세 명세: `web/WEB_FRONTEND_SPEC.md` (서버 코드 대조 완료본)

---

## [1] 팀 오너 탈퇴 시 팀 데이터 처리 🔴

**파일:** `client/src/team.c` — `team_leave()`

### 현상 (버그)

`team_leave()`는 `members` 테이블에서 본인 행만 삭제한다.
오너가 탈퇴할 경우 다음 상태가 된다:

| 테이블 | 상태 |
| --- | --- |
| `teams` | 팀 행 **그대로 남음** |
| `members` | 나머지 멤버 **그대로 남음** (dangling) |
| `logs` | 기존 로그 **그대로 남음** |
| 초대 코드 | **여전히 유효** (새 참가 가능) |

결과적으로 오너 없는 고아 팀(orphaned team)이 생성된다.

### 선택안

**A. 오너 탈퇴 차단**
- 오너는 탈퇴 불가 처리 (`team leave` 시 에러 출력)
- "팀을 해체하거나 소유권을 양도한 후 탈퇴하세요" 안내
- 소유권 양도(`team transfer <username>`) 기능 추가 필요

**B. 자동 소유권 승계**
- 오너 탈퇴 시 `joined_at` 가장 오래된 일반 멤버를 자동으로 `owner`로 승격
- 별도 명령어 불필요, UX 자연스러움
- 승계 대상 없을 경우(혼자인 경우) 팀 해체 처리 필요

**C. 단독 오너이면 팀 해체, 아니면 차단 (MVP 권장)**
- 오너가 팀에 혼자이면 → `teams`, `members`, `logs` 전체 삭제 후 탈퇴
- 다른 멤버가 있으면 → 탈퇴 차단, 에러 출력
- 소유권 양도 로직 불필요, MVP 시나리오(소규모 팀)에 현실적

### 결정
> ⬜ 미정

---

## [2] 로그인 상태에서 중복 로그인 허용 🔴

**파일:** `client/src/auth.c` — `cmd_login()`

### 현상 (버그)

`cmd_login()`이 실행 시 기존 세션 존재 여부를 확인하지 않는다.
이미 로그인된 상태에서 `devlog login`을 실행하면 아무 경고 없이 세션 파일(`~/.devlog/session`)을 덮어쓴다.

- 다른 계정으로 로그인하면 기존 세션이 조용히 교체됨
- 같은 계정으로 재로그인해도 에러 없이 통과됨

### 선택안

**A. 로그인 상태이면 차단 (권장)**
- `cmd_login()` 진입 시 `session_load()`로 세션 파일 존재 여부 확인
- 세션이 있으면 "이미 로그인 상태입니다 (사용자: %s). 먼저 logout 하세요." 출력 후 종료
- 구현 단순, 의도치 않은 세션 교체 방지

**B. 자동 로그아웃 후 재로그인 허용**
- 기존 세션을 자동으로 clear 후 새 로그인 진행
- 사용자 입력 없이 세션이 바뀌어 혼란 유발 가능

### 결정
> ⬜ 미정

---

## [3] git 디렉터리 등록 시 절대 경로 직접 입력 불편 🔴

**파일:** `client/src/git_scan.c` — `git_add()`

### 현상 (UX 불편)

`devlog git add`는 절대 경로만 인식한다.
경로를 등록할 때마다 `/home/user/projects/myapp` 형태의 전체 경로를 직접 입력해야 한다.

### 선택안

**A. 상대 경로 → 절대 경로 자동 변환 (권장)**
- `realpath()` 또는 `getcwd()` + 경로 조합으로 절대 경로 변환 후 저장
- `devlog git add .` 또는 `devlog git add ../../other-project` 형태로 사용 가능
- DB에는 항상 절대 경로로 저장하므로 기존 로직과 완전 호환

**B. 인수 생략 시 현재 디렉터리 자동 등록**
- `devlog git add` (경로 인수 없음) → `getcwd()`로 현재 디렉터리 등록
- 가장 빠른 UX: 프로젝트 디렉터리 안에서 그냥 실행

**A + B 동시 적용 (최종 권장)**
- 인수가 있으면 상대/절대 경로 모두 처리 (`realpath()`)
- 인수가 없으면 현재 디렉터리 자동 등록
- 코드 변경 범위 최소: `git_add()` 앞부분에 경로 정규화 로직만 추가

### 결정
> ⬜ 미정

---

## [4] POST /logs 중복 세션 시 500 응답 🔴

**파일:** `linuxTeam01web/.../domain/log/LogService.java` — `createLog()`

### 현상 (버그)

같은 시각(세션)에 로그가 이미 존재하면 `IllegalStateException`을 던지는데, 예외 매핑이 없어 클라이언트가 **409 대신 500**을 받는다. 웹 시드 스크립트 검증 중 확인 (2026-06-12).

### 선택안

**A. `@ExceptionHandler` 또는 `@ControllerAdvice`로 409 매핑 (권장)**
- `IllegalStateException` → 409 Conflict, `IllegalArgumentException`(150자 초과) → 400 Bad Request
- 클라이언트(C/웹)가 상태 코드로 중복 여부 판단 가능해짐

### 결정
> ⬜ 미정

---

## [5] 서버 HTTP CORS 미설정 — 웹 배포 방식 결정 필요 🔴

**파일:** `linuxTeam01web/.../auth/SecurityConfig.java`

### 현상

HTTP CORS 설정이 없어 웹 프론트가 다른 origin에서 API를 호출할 수 없다.
개발 중에는 Vite proxy(`web/vite.config.ts`)로 우회 중이지만 **배포 시에는 해결 필수**.

### 선택안

**A. 동일 origin 서빙 (권장)**
- `npm run build` 산출물(`web/dist/`)을 Spring `static/` 리소스로 포함
- CORS 설정 자체가 불필요, 배포 단순(서버 1개)

**B. 별도 호스팅 + 서버 CORS 추가**
- `SecurityConfig`에 CORS 설정 추가, 허용 origin 관리 필요

### 결정
> ⬜ 미정

---

## [6] JWT 시크릿 키 하드코딩 🔴

**파일:** `linuxTeam01web/.../auth/JwtUtil.java`

### 현상 (보안)

`SECRET_KEY`가 소스 코드에 하드코딩되어 저장소에 노출된다. 만료 시간(24h)도 상수 고정.

### 선택안

**A. 환경 변수/`application.yml` 외부화 (권장)**
- `@Value("${devlog.jwt.secret}")` + 배포 환경에서 환경 변수 주입
- 만료 시간도 설정으로 이동

### 결정
> ⬜ 미정 (MVP 데모까지는 현행 유지 가능)

---

## [7] 웹 프론트 P2 개선 후보 🟡

**파일:** `web/src/` 전반

배포 전 필수는 아니며, 시간 여유 시 진행:

| 항목 | 내용 |
| --- | --- |
| 폴링 실검증 | 오늘 화면에서 60초 내 신규 로그 자동 반영 확인 (다음 정각 이후 시드 재실행) |
| 모바일 점검 | 360px에서 그리드 가로 스크롤·터치 동작 시각 확인 |
| 컴포넌트 테스트 | `TimelineGrid` 셀 매핑·선택 토글 테스트 추가 (현재 유틸/매퍼 12개만) |
| 세션 만료 UX | 401 리다이렉트 시 "세션이 만료되었습니다" 안내 문구 |
| STOMP 실시간 | 라이브러리(`@stomp/stompjs`) 도입 재결정 시 폴링 대체 (서버 `/ws` 준비됨) |
| 서버 API 보강 연동 | 로그 응답에 `username`, 팀/멤버에 `createdAt`/`joinedAt` 추가되면 웹 매퍼 단순화 + 화면 보강 (명세서 5장 동시 갱신) |

### 결정
> 🟡 우선순위 논의 후 진행
