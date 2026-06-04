## Devlog Project

터미널에서 작업 상태·Git 활동·시스템 상태를 1시간 단위로 기록하고, 팀원과 공유하는 Linux CLI 도구.

개발자들끼리 본인들의 개발 상황을 재미있게 공유하는 SNS 서비스.

---

## 실행 방법 (Linux)

### 0. Clone

```bash
git clone https://github.com/EunjinJeong26/linuxTeam01.git
cd linuxTeam01
```

### 1. 의존성 설치 (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y gcc make libcurl4-openssl-dev libsqlite3-dev openjdk-21-jdk
```

### 2. 서버 먼저 기동

```bash
cd linuxTeam01web
chmod +x gradlew
./gradlew bootRun
```

포트 8080에서 실행됩니다. `devlog.sqlite` 파일이 자동 생성됩니다.
**별도 터미널에서 켜두고 아래 단계를 진행하세요.**

> Java 21이 인식되지 않으면: `export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64`

### 3. 클라이언트 빌드

```bash
cd client
make
```

`devlog`, `devlogd` 두 바이너리가 생성됩니다.

### 4. 기본 사용법

```bash
./devlog register              # 회원가입
./devlog login                 # 로그인
./devlog git add .             # 현재 디렉토리를 git 추적 경로로 등록
./devlog post                  # 로그 작성 (sysinfo + git 자동 수집)
./devlog show                  # 팀 전체 로그 조회
./devlog show --my             # 내 로그만 조회
./devlog team create <팀명>    # 팀 생성
./devlog team join <초대코드>  # 팀 참가
```

### 5. 데몬 (선택 — 정각 알림)

```bash
# 빠른 테스트: 직접 실행
sudo ./devlogd &

# systemd 등록:
sudo install -m 755 devlogd /usr/local/bin/devlogd
sudo cp ../daemon/devlog.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now devlogd

./devlog notify on   # 알림 활성화
```

---

### md 파일 설명

- CLAUDE.md: Claude Code 컨텍스트 유지 파일 (개발용)
- RULES.md : Claude Code 컨텍스트 세부 내용, 규칙 파일 (개발용)
- PLAN.md : Devlog 기획서
- DECISIONS.md : PLAN.md에서 의사결정 로그 부분 분리하여 따로 표시 (토큰 절약)
- DEMO.md : 로컬 프로토타입 테스트 방법 가이드
- IMPLEMENTS.md : 구현 상황 기록 + 버그 & 개선안 작성
