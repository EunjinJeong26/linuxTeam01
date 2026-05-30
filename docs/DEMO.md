# Devlog 데모 테스트 가이드

## 사전 준비 (최초 1회)

WSL(Ubuntu) 터미널에서 의존성 설치:

```bash
sudo apt update && sudo apt install -y gcc make libsqlite3-dev libcurl4-openssl-dev
```

## 빌드

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
