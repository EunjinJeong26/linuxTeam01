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
