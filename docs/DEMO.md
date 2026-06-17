# Devlog 데모 테스트 가이드

## 사전 준비 (최초 1회)

WSL(Ubuntu) 터미널에서 의존성 설치:

```bash
sudo apt update && sudo apt install -y gcc make libsqlite3-dev libcurl4-openssl-dev
```

## 0. 서버 기동 (가장 먼저)

클라이언트의 `API_BASE_URL`이 `http://localhost:8080`을 바라보므로, 아래 테스트 전에 Spring Boot 서버를 먼저 띄워야 합니다.

```powershell
cd linuxTeam01web
.\gradlew.bat bootRun
```

`localhost:8080`에서 SQLite(`devlog.sqlite`) 기반으로 기동됩니다. (C 클라이언트는 WSL, 서버는 Windows에서 실행해도 같은 머신이면 `localhost`로 통신 가능)

## 빌드 (C 클라이언트)

```bash
cd client
make
```

빌드 성공 시 `client/devlog` 바이너리가 생성됩니다.

---

## 테스트 순서

### 1. 사용자 A — 등록 · 팀 생성

```bash
./devlog register       # username / password 입력
./devlog login
./devlog whoami         # 로그인 확인

./devlog team create DevTeam
./devlog team info      # 초대 코드 메모
```

### 2. 사용자 A — git 등록 · 로그 작성

```bash
./devlog git add /절대/경로/프로젝트
./devlog git list       # 등록 확인

./devlog post           # 코멘트 입력 → CPU/MEM/Git 자동 수집
./devlog show           # 오늘 팀 현황 확인
```

### 3. 사용자 B — 등록 · 팀 참가 · 로그 작성

```bash
./devlog logout
./devlog register       # 다른 username으로 등록
./devlog login

./devlog team join XXXXXXXX   # 위에서 메모한 초대 코드
./devlog post
./devlog show           # A · B 두 명의 로그 동시 출력 확인
```

### 4. 추가 기능 확인

```bash
./devlog show --my                  # 내 로그만 조회
./devlog show --date 2026-05-24     # 날짜 지정 조회
./devlog team leave                 # 팀 탈퇴
```

---

## 5. 웹 프론트엔드 확인 (선택, M1~M5 읽기 전용 뷰어)

서버가 0번 단계로 떠 있는 상태에서 진행합니다.

```powershell
cd web
npm install              # 최초 1회
.\scripts\seed-dev.ps1   # jin/minseo 계정 + 팀 + 로그 2건 시드
npm run dev
```

브라우저에서 `localhost:5173` 접속 → `jin / pass1234`로 로그인 → 팀 로그 화면에 jin·minseo 로그가 보이는지 확인합니다.
단위 테스트만 돌리려면 `npm run test` (vitest).

## 6. 교차 검증

- **C 클라이언트로 `post`한 로그**가 웹 프론트 화면에도 보이는지 확인 (클라이언트·서버·프론트가 같은 DB로 묶여 있는지 검증하는 핵심 시나리오).
- 서버를 재시작해도 `devlog.sqlite`에 데이터가 남아있는지 확인 (영속성 검증).
