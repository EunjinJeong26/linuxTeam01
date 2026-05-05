# 개발일기 CLI - 팀원 가이드

## 목차
1. [구조 요약](#1-구조-요약)
2. [빌드 방법](#2-빌드-방법)
3. [로컬 테스트 방법](#3-로컬-테스트-방법)
4. [팀 서버 배포 방법](#4-팀-서버-배포-방법)
5. [팀원 구현 가이드](#5-팀원-구현-가이드)
6. [데이터 저장 구조](#6-데이터-저장-구조)

---

## 1. 구조 요약

### CLI 커맨드 (git 스타일)
```bash
diary register    # 회원가입
diary login       # 로그인
diary logout      # 로그아웃
diary write       # 일기 쓰기 (로그인 필요)
diary read        # 내 일기 목록 (로그인 필요)
diary public      # 공개 일기 목록
diary whoami      # 현재 로그인 확인
diary help        # 도움말
```

### 핵심 설계
- **다이어리 앱 자체 회원 시스템** 사용 — Linux 계정과 무관
- 회원 정보: `data/users.txt` (id|username|password|nickname|createdAt)
- 로그인 상태: `~/.diary_session` (각자 PC/계정에 개별 저장)
- 공유 데이터: `~/diary/` (팀 서버에서 같은 계정으로 SSH 접속 시 자동 공유)

### 추가된 파일
| 파일 | 역할 |
|------|------|
| `src/session.h/c` | `~/.diary_session`으로 로그인 상태 유지 |

---

## 2. 빌드 방법

```bash
cd c-program
make clean && make
./diary help   # 실행 확인
```

---

## 3. 로컬 테스트 방법

별도 설정 없이 바로 실행 가능해요. 데이터는 `~/diary/`에 자동 생성돼요.

```bash
./diary help
./diary register
./diary login
./diary write
./diary read
```

### 단계별 테스트 시나리오

#### Step 1. 기본 동작 확인 (구현 전)
```bash
./diary           # 도움말 출력 확인
./diary whoami    # "로그인 상태 아님" 확인
./diary write     # "로그인 필요" 에러 확인
./diary asdf      # "알 수 없는 커맨드" 확인
```

#### Step 2. 전체 흐름 테스트 (팀원 구현 완료 후)
```bash
./diary register          # 회원가입
./diary login             # 로그인
./diary whoami            # 로그인된 사용자 확인
./diary write             # 일기 작성
./diary read              # 내 일기 목록
./diary public            # 공개 일기 목록
./diary logout            # 로그아웃
./diary whoami            # "로그인 상태 아님" 재확인
```

#### Step 3. 파일 직접 확인
```bash
cat ~/.diary_session              # 세션 파일 내용
cat ~/diary/data/users.txt       # 가입된 사용자 목록
ls ~/diary/data/diaries/         # 일기 폴더
```

---

## 4. 팀 서버 배포 방법

팀원 모두 `team01` 같은 **같은 계정**으로 SSH 접속하면 `~/diary/`가 자동으로 공유돼요.
별도 경로 설정이나 루트 권한이 필요 없어요.

```bash
# ── 코드 받기 & 빌드 ──
git clone https://github.com/EunjinJeong26/linuxTeam01.git
cd linuxTeam01/c-program
sudo apt install -y build-essential  # gcc 없으면 설치
make

# ── PATH 등록 (한 번만) ──
sudo ln -sf $(pwd)/diary /usr/local/bin/diary

# ── 확인 ──
diary help
```

### 코드 업데이트 시
```bash
cd linuxTeam01
git pull origin spring
cd c-program && make
# PATH 심볼릭 링크는 그대로 유지되므로 make만 하면 됨
```

---

## 5. 팀원 구현 가이드

### 공통 주의사항
- `DATA_DIR`, `DIARIES_DIR`, `USERS_FILE` 상대경로 **그대로 사용** OK
  (`init_data_dir()`이 `~/diary/`로 `chdir()`해주기 때문)
- 경로 조합 시 `build_diary_dir_path()`, `build_diary_file_path()` **반드시 사용**
- 경로 직접 하드코딩 금지 (`"data/diaries/eunji"` 같은 것)
- `c-program/data/`는 git 구조 유지용 빈 폴더 — **실제 런타임 데이터는 여기 저장되지 않음**
  실제 데이터 위치: `~/diary/data/users.txt`, `~/diary/data/diaries/{username}/`

---

### 유해린 — user.c 구현

#### 구현할 함수
```c
int register_user(void);
int login_user(User *out_user);
int is_username_taken(const char *username);
int save_user(const User *user);
```

#### users.txt 형식
```
1|eunji|1234|은지|2026-05-03T10:00:00
2|haerin|abcd|해린|2026-05-04T09:00:00
```
> 헤더 행 없음. 첫 줄부터 바로 데이터. `sscanf(line, "%d|%s...")`로 파싱.

#### 구현 포인트
```c
// USERS_FILE 상대경로 그대로 사용 (init_data_dir이 chdir 해줌)
FILE *fp = fopen(USERS_FILE, "r");  // 실제로는 ~/diary/data/users.txt

// login_user: 반드시 out_user 채워서 반환
// main.c에서 save_session(out_user)로 세션 저장에 사용됨
int login_user(User *out_user) {
    // users.txt 파싱 후 username/password 비교
    // 성공 시: out_user->id, username, nickname 채우기
    // 반환: RET_OK 또는 RET_FAIL
}
```

#### 테스트
```bash
./diary register   # username, password, nickname 입력
./diary login      # 로그인 → ~/.diary_session 생성 확인
./diary whoami     # 세션에서 읽은 사용자 정보 출력
cat ~/diary/data/users.txt   # 저장된 회원 정보 확인
```

---

### 정은진 — diary_write.c 구현

#### 구현할 함수
```c
int write_diary(const User *author);
void generate_diary_filename(const char *username, char *out, int out_size);
int save_diary(const Diary *diary, const char *username);
```

#### 일기 파일 형식 (data/diaries/{username}/YYYY-MM-DD_NNN.txt)
```
id=2026-05-03_001
author=eunji
title=오늘 일기
visibility=PRIVATE
createdAt=2026-05-03T14:30:00
updatedAt=2026-05-03T14:30:00
content=오늘은 수업을 듣고 팀플 회의를 했다.
```

#### 구현 포인트
```c
int write_diary(const User *author) {
    // 1. 사용자 일기 폴더 확인/생성
    char dir[MAX_PATH_LEN];
    build_diary_dir_path(author->username, dir, sizeof(dir));
    make_dir(dir);  // fileio.h의 make_dir 사용

    // 2. 제목, 내용, 공개 여부 입력받기
    // 3. Diary 구조체 채우기
    //    visibility: VISIBILITY_PUBLIC 또는 VISIBILITY_PRIVATE 사용
    // 4. save_diary()로 파일 저장
}
```

#### 테스트
```bash
./diary login
./diary write     # 제목, 내용, 공개여부 입력
ls ~/diary/data/diaries/{username}/   # 파일 생성 확인
cat ~/diary/data/diaries/{username}/2026-05-04_001.txt
```

---

### 김규린 — diary_read.c 구현

#### 구현할 함수
```c
int list_my_diaries(const User *user);
int list_public_diaries(void);
int read_diary(const char *username, const char *filename, const User *viewer);
```

#### 구현 포인트
> **주의**: `content` 줄은 최대 4096자(`MAX_CONTENT_LEN`)임.
> `fgets(line, MAX_LINE_LEN, fp)` 대신 `fgets(line, MAX_CONTENT_LEN, fp)` 사용할 것.

```c
int list_my_diaries(const User *user) {
    char dir[MAX_PATH_LEN];
    build_diary_dir_path(user->username, dir, sizeof(dir));
    // opendir/readdir로 dir 안의 .txt 파일 목록 출력
}

int list_public_diaries(void) {
    // DIARIES_DIR 아래 모든 사용자 폴더 순회
    // visibility=PUBLIC 인 일기만 출력
    // 진짜 교환일기처럼 다른 사람 글도 보임
}

int read_diary(..., const User *viewer) {
    // visibility=PRIVATE이고 viewer != author면 접근 거부
}
```

#### 테스트
```bash
./diary public          # 로그인 없이 공개 일기 목록
./diary login
./diary read            # 내 일기 목록
```

---

## 6. 데이터 저장 구조

```
~/diary/
├── data/
│   ├── users.txt           ← 모든 사용자 공유 (diary 앱 자체 회원 DB)
│   └── diaries/
│       ├── eunji/          ← eunji 계정의 일기
│       └── haerin/         ← haerin 계정의 일기

~/.diary_session             ← 각자 로그인 세션 (Linux 계정별 개별 저장)
```

> **핵심**: Linux 계정(`ssh user@server`)과 diary 앱 계정(`diary register`)은 별개.
> 팀 서버에서 같은 Linux 계정으로 접속하면 `~/diary/`가 동일한 경로이므로
> 데이터가 자동으로 공유돼서 진짜 교환일기처럼 동작해요.
