# Devlog Architecture & Implementation Rules

이 문서는 클라이언트 및 데몬 개발 시 반드시 지켜야 할 기술적 제약 사항입니다.

## 1. Client-Daemon Communication (Unix Domain Socket)
- CLI 클라이언트(`main.c`)와 백그라운드 데몬(`devlogd.c`)은 **반드시 Unix Domain Socket (UDS)**을 통해 통신한다.
- 데몬은 root 권한으로 실행됨을 가정하며, 알림 이벤트 발생 시 터미널 벨(`\a`)을 출력해야 한다.
- 알림 토글(`devlog notify on/off`) 명령은 CLI가 UDS를 통해 데몬의 상태를 변경하는 방식으로 구현한다.

## 2. Server API Communication (libcurl)
- 서버와의 통신은 `libcurl`을 이용한 REST API 호출로 한정한다.
- 데이터 직렬화/역직렬화는 `cJSON` 라이브러리를 사용한다.
- 인증은 JWT를 사용한다. 로그인 성공 시 발급받은 토큰은 클라이언트 로컬(`~/.devlog_session`)에 평문으로 저장하고, 이후 모든 `libcurl` 요청 헤더(`Authorization: Bearer <token>`)에 포함해야 한다.

## 3. Data Collection Constraints (sysinfo & git)
- **CPU/Memory**: `/proc/stat` 및 `/proc/meminfo`를 파싱하여 '호출 순간'의 스냅샷 데이터를 추출한다. (평균치 계산 금지)
- **Git Commit Scan**: 
  - 외부 API(GitHub) 절대 사용 금지.
  - 사용자가 등록한 디렉토리 목록(CLI `devlog git list`)의 하위 트리를 재귀적으로 탐색하여 `.git` 폴더를 찾는다.
  - 다수의 `.git` 발견 시 가장 최근 커밋 시간을 가진 레포지토리의 정보를 우선 수집한다.

## 4. Local File System Structure
- 사용자의 세션 및 캐시 데이터는 홈 디렉토리 하위를 기준으로 다음과 같이 관리한다. 이 경로를 하드코딩하지 말고 환경변수 `HOME`을 참조하여 동적으로 경로를 구성하라.
  - `~/devlog/data/users.txt`
  - `~/devlog/data/teams.txt`
  - `~/devlog/data/members.txt`
  - `~/devlog/data/logs/<username>/<date>_<hour>.txt`
  - `~/.devlog_session`

## 5. Output & UI
- CLI 명령어 출력 결과는 가독성을 위해 간결해야 하며, 성공/실패 여부를 명확히 표준 출력(`stdout`) 및 표준 에러(`stderr`)로 구분하여 출력한다.
- 텍스트 입력의 경우 한국어 기준 최대 150자 제한 로직을 클라이언트 단에서 1차 검증한다.