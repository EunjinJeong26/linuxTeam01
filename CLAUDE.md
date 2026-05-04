# 개발일기 시스템 (Development Diary System)

## 프로젝트 개요
개발자를 위한 CLI 기반 교환일기 시스템.
터미널에서 일기를 쓰고, 다른 개발자들과 공유할 수 있는 공개형 교환일기.
Git처럼 CLI에서 직접 접근 가능하게 설계.

## 기술 스택
- **C** — CLI 프로그램 (시스템 콜, 파일 I/O)
- **Linux (Ubuntu)** — 실행 환경, 파일시스템이 DB 역할
- **Java Spring Boot** — REST API 서버 (향후 연결)
- **React** — 웹 프론트엔드 (향후 연결)

## 개발 단계 및 최종 목표 아키텍처

### 단계별 로드맵
```
[1단계 - 현재] C CLI → ~/diary/ 공유 파일 직접 읽기/쓰기
               (팀 서버에 같은 계정으로 SSH 접속하면 누구든 같은 데이터 공유)
[2단계 - 다음] C CLI → HTTP 요청 → Spring Boot API → 파일/DB
               (SSH 없이 어느 컴퓨터에서든 접속 가능)
[3단계 - 최종] React 웹도 동일 API 사용
```

### 최종 목표 구조 (git처럼 어디서든 접속)
```
[내 컴퓨터 CMD]              [공유 서버]               [다른 컴퓨터 CMD]
  diary write  → HTTP POST → Spring Boot API ← HTTP GET ← diary read
  diary login  → HTTP POST → (인증 처리)
                           → ~/diary/data/ (데이터 저장)
                           ← React 웹도 동일하게 연결
```

> **핵심**: PATH 등록은 같은 머신에서만 동작.
> 진짜 "어느 컴퓨터에서든 접속"하려면 Spring Boot 서버가 필요.
> 지금 만든 C CLI의 파일 I/O 부분을 나중에 HTTP 요청으로 교체하면 됨.

### 팀 서버에서 바로 실행하는 방법 (1단계)
팀원 모두 같은 Linux 계정(예: `team01`)으로 SSH 접속하면 `~/diary/`가 자동 공유됨.
별도 경로 설정이나 루트 권한 불필요.

```bash
# 코드 받기 & 빌드
git clone https://github.com/EunjinJeong26/linuxTeam01.git
cd linuxTeam01/c-program && make
sudo ln -sf $(pwd)/diary /usr/local/bin/diary
diary help   # 완료
```

## CLI 사용법 (git 스타일 서브커맨드)
```bash
diary register    # 회원가입
diary login       # 로그인  → ~/.diary_session 생성
diary logout      # 로그아웃 → ~/.diary_session 삭제
diary write       # 일기 작성 (로그인 필요)
diary read        # 내 일기 목록 (로그인 필요)
diary public      # 공개 일기 목록 (로그인 불필요)
diary whoami      # 현재 로그인 사용자 확인
diary help        # 도움말
```

## 레포 구조
```
linuxTeam01/
├── c-program/
│   ├── src/
│   │   ├── main.c          - 서브커맨드 파싱/디스패치 (진공 담당)
│   │   ├── common.h        - 공통 구조체/상수 (진공 담당)
│   │   ├── fileio.c/h      - 파일/폴더 초기화, ~/diary/ chdir (진공 담당)
│   │   ├── session.c/h     - 로그인 세션 관리 ~/.diary_session (진공 담당)
│   │   ├── user.c/h        - 회원가입/로그인 (유해린 담당)
│   │   ├── diary_write.c/h - 일기 작성/저장 (정은진 담당)
│   │   └── diary_read.c/h  - 일기 조회/권한 (김규린 담당)
│   ├── data/               - .gitignore 처리 (런타임에 ~/diary/data/ 로 자동 생성)
│   ├── GUIDE.md            - 팀원 구현 가이드 및 테스트 방법
│   └── Makefile
└── team01web/              - Spring Boot 프로젝트
```

## 역할 분담
| 담당자 | 파일 | 기능 |
|--------|------|------|
| 진공 (김재하) | main.c, common.h, fileio.c/h, session.c/h | 메인/공통/파일I/O/세션, 전체 통합 |
| 유해린 | user.c/h | 회원가입, 로그인, users.txt 관리 |
| 정은진 | diary_write.c/h | 일기 작성, 파일명 생성, 저장 |
| 김규린 | diary_read.c/h | 일기 조회, PUBLIC/PRIVATE 권한 |

## 데이터 형식

### users.txt
```
1|eunji|1234|은지|2026-05-03T10:00:00
```
> 헤더 행 없음. 위는 형식 안내용이며 실제 파일에는 데이터만 저장됨.

### 일기 파일 (data/diaries/{username}/YYYY-MM-DD_NNN.txt)
```
id=2026-05-03_001
author=eunji
title=오늘 일기
visibility=PRIVATE
createdAt=2026-05-03T14:30:00
updatedAt=2026-05-03T14:30:00
content=오늘은 수업을 듣고 팀플 회의를 했다.
```

### 세션 파일 (~/.diary_session)
```
id=1
username=eunji
nickname=은지
```

## 데이터 저장 위치

```
~/diary/data/users.txt        ← 다이어리 앱 자체 회원 DB (Linux 계정과 무관)
~/diary/data/diaries/eunji/   ← eunji 계정 일기
~/diary/data/diaries/haerin/  ← haerin 계정 일기
~/.diary_session               ← 각자 로그인 세션 (Linux 계정별 개별 저장)
```

- `init_data_dir()`이 `~/diary/`로 `chdir()` — 이후 모든 상대경로가 자동으로 올바른 위치를 가리킴
- **팀원 코드 변경 불필요**
- 팀 서버에서 같은 Linux 계정으로 SSH 접속 시 `~/diary/`가 동일 경로 → 데이터 자동 공유

> 다른 경로를 쓰고 싶을 때만: `export DIARY_DATA_DIR=/원하는/경로`

## 빌드 및 실행

### WSL / Linux에서 빌드
```bash
cd c-program
make clean && make
./diary help
```

### Linux 서버에서 clone 후 실행
```bash
git clone https://github.com/EunjinJeong26/linuxTeam01.git
cd linuxTeam01/c-program
sudo apt install -y build-essential
make
sudo ln -sf $(pwd)/diary /usr/local/bin/diary
diary help
```

## 현재 진행 상태 (2026-05-04 기준)
- [x] 프로젝트 기획 완료
- [x] 폴더 구조 및 기초 파일 생성 완료
- [x] common.h — User/Diary 구조체, 공통 상수, 반환 코드 정의 완료
- [x] fileio.c/h — 데이터 폴더 초기화 (~/diary/ chdir) 완료
- [x] session.c/h — 로그인 세션 관리 (~/.diary_session) 완료
- [x] main.c — git 스타일 서브커맨드 CLI 완료
- [x] Makefile, .gitignore 작성 완료
- [x] GUIDE.md — 팀원 구현 가이드 및 테스트 방법 작성
- [x] WSL 빌드 및 실행 확인 (`diary help`, `diary whoami` 등)
- [ ] user.c — 회원가입/로그인 구현 (유해린)
- [ ] diary_write.c — 일기 작성 구현 (정은진)
- [ ] diary_read.c — 일기 조회 구현 (김규린)
- [ ] 팀원 파일 통합 테스트
- [ ] 팀 서버 배포 (SSH → make → PATH 등록)
- [ ] Spring Boot API 연동 (2단계)

## 개발 환경 (2026-05-04 기준)
- OS: Windows + WSL (Ubuntu)
- IDE: VS Code + WSL 익스텐션 + Claude Code 익스텐션
- 빌드: WSL 터미널에서 make 실행
- WSL 경로: /mnt/c/Users/fullr/바탕화면/Coding/Linux/linuxTeam01/c-program

## GitHub
- 레포: https://github.com/EunjinJeong26/linuxTeam01
- 기본 브랜치: spring
- C 파트 경로: c-program/

## 주의사항
- C 코드는 Linux 전용 헤더 사용 → WSL 또는 Linux 서버에서만 컴파일/실행 가능
- data/ 폴더는 .gitignore 처리됨 (런타임에 ~/diary/data/로 자동 생성)
- 팀원 파일(user.c, diary_write.c, diary_read.c)은 TODO 껍데기 상태 — 각자 구현 후 PR
- 팀원 구현 시 c-program/GUIDE.md 반드시 참고
- PATH 등록은 같은 머신에서만 유효 — 다른 컴퓨터 접속은 SSH 또는 Spring Boot API 필요

## 초기 기획 대비 변경사항

### 1. main.c — 실행 방식 변경
| 항목 | 초기 기획 | 변경 후 |
|------|-----------|---------|
| 실행 방식 | 실행 후 터미널 내 메뉴 선택 (while 루프) | git 스타일 서브커맨드 (`diary write`, `diary login` 등) |
| 이유 | 서브커맨드 방식이 PATH 등록 후 어디서든 바로 사용 가능하고 스크립트 자동화에도 적합 |

### 2. 데이터 저장 위치 변경
| 항목 | 초기 기획 | 변경 후 |
|------|-----------|---------|
| 저장 경로 | `c-program/data/diaries/` (소스 폴더 기준 상대경로) | `~/diary/data/diaries/` (홈 디렉터리 기준 고정 경로) |
| 이유 | PATH 등록 후 어느 폴더에서 실행해도 항상 같은 위치에 저장되어야 함. 소스 상대경로는 실행 위치에 따라 경로가 달라짐 |

### 3. 세션 관리 추가 (신규)
| 항목 | 초기 기획 | 변경 후 |
|------|-----------|---------|
| 로그인 상태 유지 | 없음 (매 실행마다 입력) | `~/.diary_session` 파일로 로그인 상태 유지 |
| 추가 파일 | — | `src/session.c`, `src/session.h` |
| 이유 | 서브커맨드 방식에서는 각 명령어가 별도 프로세스로 실행되기 때문에 세션 파일 없이는 로그인 상태를 유지할 수 없음 |

### 4. 데이터 폴더명 변경
| 항목 | 초기 기획 | 변경 후 |
|------|-----------|---------|
| 폴더명 | `~/.diary/` (숨김 폴더) | `~/diary/` (일반 폴더) |
| 이유 | 팀원들이 직접 데이터 파일을 확인해야 하는 경우가 많아 숨김 폴더보다 접근이 쉬운 일반 폴더가 적합 |
