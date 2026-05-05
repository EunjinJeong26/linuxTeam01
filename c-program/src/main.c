/**
 * main.c - git 스타일 서브커맨드 CLI 진입점
 *
 * 사용법: diary <커맨드>
 *
 * 커맨드:
 *   register   회원가입
 *   login      로그인 (세션 파일 생성: ~/.diary_session)
 *   logout     로그아웃 (세션 파일 삭제)
 *   write      일기 작성 (로그인 필요)
 *   read       내 일기 목록 (로그인 필요)
 *   public     공개 일기 목록 (로그인 불필요)
 *   whoami     현재 로그인 사용자 확인
 *   help       도움말
 */

#include "common.h"
#include "fileio.h"
#include "session.h"
#include "user.h"
#include "diary_write.h"
#include "diary_read.h"

static void print_help(void) {
    printf("\n개발일기 시스템 (CLI)\n");
    printf("----------------------------------\n");
    printf("사용법: diary <커맨드>\n\n");
    printf("커맨드:\n");
    printf("  register   회원가입\n");
    printf("  login      로그인\n");
    printf("  logout     로그아웃\n");
    printf("  write      일기 쓰기          (로그인 필요)\n");
    printf("  read       내 일기 목록 보기  (로그인 필요)\n");
    printf("  public     공개 일기 보기\n");
    printf("  whoami     현재 로그인 사용자 확인\n");
    printf("  help       이 도움말 출력\n");
    printf("\n예시:\n");
    printf("  diary register\n");
    printf("  diary login\n");
    printf("  diary write\n");
    printf("  diary read\n");
    printf("  diary logout\n\n");
}

/* 로그인이 필요한 커맨드 실행 전 세션 확인 */
static int require_login(User *out_user) {
    if (load_session(out_user) == RET_FAIL) {
        fprintf(stderr, "로그인이 필요합니다. 먼저 아래 명령어를 실행하세요:\n");
        fprintf(stderr, "  diary login\n");
        return RET_FAIL;
    }
    return RET_OK;
}

static int cmd_register(void) {
    return register_user();
}

static int cmd_login(void) {
    if (is_logged_in()) {
        User cur;
        load_session(&cur);
        printf("이미 로그인 중입니다: %s (%s)\n", cur.nickname, cur.username);
        printf("다른 계정으로 로그인하려면 먼저 'diary logout'을 실행하세요.\n");
        return RET_OK;
    }

    User user;
    if (login_user(&user) == RET_OK) {
        if (save_session(&user) == RET_OK) {
            printf("로그인 성공! 안녕하세요, %s 님.\n", user.nickname);
            return RET_OK;
        }
        fprintf(stderr, "세션 저장에 실패했습니다.\n");
        return RET_FAIL;
    }

    fprintf(stderr, "로그인 실패. 아이디 또는 비밀번호를 확인하세요.\n");
    return RET_FAIL;
}

static int cmd_logout(void) {
    if (!is_logged_in()) {
        printf("현재 로그인 상태가 아닙니다.\n");
        return RET_OK;
    }
    User cur;
    load_session(&cur);
    if (clear_session() == RET_OK) {
        printf("로그아웃했습니다. 안녕히 가세요, %s 님.\n", cur.nickname);
        return RET_OK;
    }
    fprintf(stderr, "로그아웃 중 오류가 발생했습니다.\n");
    return RET_FAIL;
}

static int cmd_write(void) {
    User user;
    if (require_login(&user) == RET_FAIL) return RET_FAIL;
    return write_diary(&user);
}

static int cmd_read(void) {
    User user;
    if (require_login(&user) == RET_FAIL) return RET_FAIL;
    return list_my_diaries(&user);
}

static int cmd_public(void) {
    return list_public_diaries();
}

static int cmd_whoami(void) {
    User user;
    if (load_session(&user) == RET_FAIL) {
        printf("현재 로그인 상태가 아닙니다.\n");
        printf("  diary login  으로 로그인하세요.\n");
        return RET_FAIL;
    }
    printf("현재 로그인: %s (닉네임: %s, ID: %d)\n",
           user.username, user.nickname, user.id);
    return RET_OK;
}

int main(int argc, char *argv[]) {
    /* 1. ~/.diary/ 생성 및 chdir - 이후 상대경로 모두 여기 기준 */
    if (init_data_dir() == RET_FAIL) {
        fprintf(stderr, "데이터 폴더 초기화 실패\n");
        return 1;
    }

    /* 2. 커맨드 없이 실행하면 도움말 출력 */
    if (argc < 2) {
        print_help();
        return 0;
    }

    /* 3. 서브커맨드 디스패치 */
    const char *cmd = argv[1];

    if      (strcmp(cmd, "register") == 0) return cmd_register();
    else if (strcmp(cmd, "login")    == 0) return cmd_login();
    else if (strcmp(cmd, "logout")   == 0) return cmd_logout();
    else if (strcmp(cmd, "write")    == 0) return cmd_write();
    else if (strcmp(cmd, "read")     == 0) return cmd_read();
    else if (strcmp(cmd, "public")   == 0) return cmd_public();
    else if (strcmp(cmd, "whoami")   == 0) return cmd_whoami();
    else if (strcmp(cmd, "help")     == 0) { print_help(); return 0; }
    else {
        fprintf(stderr, "알 수 없는 커맨드: '%s'\n\n", cmd);
        print_help();
        return 1;
    }
}
