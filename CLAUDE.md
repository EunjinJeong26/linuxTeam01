# Devlog Project - Claude Code Instructions

## 1. Project Overview
- **Type**: C 기반 CLI/TUI SNS 클라이언트 및 Spring Boot 백엔드
- **OS Target**: ONLY Linux (Ubuntu). macOS/WSL 호환성 무시.
- **Goal**: 기획서(PLAN.md)와 규칙(RULES.md)에 기반한 빠르고 정확한 기능 구현.
- **Rule of Thumb**: 명시된 구조를 임의로 변경하지 말고, 추가 패키지나 의존성을 도입할 때는 반드시 먼저 물어볼 것.

## 2. Tech Stack & Dependencies
- **Client (C)**: `gcc`, `libcurl` (REST API), `strstr` 기반 경량 파싱 (cJSON 미사용), `ncurses` (TUI 적용 시)
- **Daemon (C)**: Systemd service, Unix Domain Socket (UDS), Cron/Timer
- **Server (Java)**: Spring Boot 3.x, SQLite (JDBC), JWT Auth

## 3. Build & Run Commands (C Client)
- **Build**: `make` (루트 디렉토리의 Makefile 사용)
- **Clean**: `make clean`
- **Run Client**: `./client/build/devlog [command]`
- **Run Daemon**: `sudo systemctl start devlogd` (설치된 경우)

## 4. AI Behavior & Token Optimization Rules
1. **Do not guess**: 기획이나 아키텍처에 대한 의문이 생기면 `PLAN.md`와 `RULES.md`를 우선 확인하라.
2. **Read minimal files**: 작업 지시를 받으면 프로젝트 전체를 스캔하지 말고, 관련된 `.c`, `.h` 파일만 정확히 타겟팅하여 읽어라 (예: 로그인 기능 구현 시 `auth.c/h`, `api.c/h`만 확인).
3. **Write concise code**: C 코드는 POSIX 표준을 준수하며, 불필요한 주석을 길게 달지 마라.
4. **No external APIs**: GitHub API 등 외부 인터넷 연동은 절대 사용하지 않는다.
5. **No mock data without context**: UI/로직 테스트 시 반드시 `~/devlog/data/` 구조에 맞는 로컬 텍스트 파일을 기반으로 테스트하라.

## 5. Language Rules
- **모든 응답은 한글로 작성**한다. 코드 외의 모든 설명, 질문, 피드백은 반드시 한글로 표시하라.

## 6. Reference Files
- 전체 기획 및 기획 변동 내역: `/PLAN.md`
- 상세 구현 제약 및 아키텍처 룰: `/RULES.md`