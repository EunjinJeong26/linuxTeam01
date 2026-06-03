#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "auth.h"
#include "api.h"
#include "session.h"
#include "common.h"

/* fgets로 읽은 버퍼에서 맨 끝 개행 제거 */
static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
}

int cmd_register(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    printf("username: ");
    fflush(stdout);
    if (fgets(username, sizeof(username), stdin) == NULL) return -1;
    strip_newline(username);

    printf("password: ");
    fflush(stdout);
    if (fgets(password, sizeof(password), stdin) == NULL) return -1;
    strip_newline(password);

    int rc = api_register(username, password);
    if (rc == -2) {
        fprintf(stderr, "이미 존재하는 사용자명입니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "회원가입 요청 실패\n");
        return -1;
    }

    printf("등록 완료: %s\n", username);
    return 0;
}

int cmd_login(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    printf("username: ");
    fflush(stdout);
    if (fgets(username, sizeof(username), stdin) == NULL) return -1;
    strip_newline(username);

    printf("password: ");
    fflush(stdout);
    if (fgets(password, sizeof(password), stdin) == NULL) return -1;
    strip_newline(password);

    Session s;
    if (api_login(username, password, s.token, sizeof(s.token)) != 0) {
        fprintf(stderr, "사용자명 또는 비밀번호가 올바르지 않습니다.\n");
        return -1;
    }

    strncpy(s.username, username, MAX_USERNAME_LEN - 1);
    s.username[MAX_USERNAME_LEN - 1] = '\0';

    if (session_save(&s) != 0) return -1;

    printf("로그인 완료: %s\n", username);
    return 0;
}

int cmd_logout(void) {
    Session s;
    if (session_load(&s) != 0) {
        fprintf(stderr, "로그인 상태가 아닙니다.\n");
        return -1;
    }

    session_clear();
    printf("로그아웃 완료\n");
    return 0;
}

int cmd_whoami(void) {
    Session s;
    if (session_load(&s) != 0) {
        fprintf(stderr, "로그인 상태가 아닙니다.\n");
        return -1;
    }

    printf("%s\n", s.username);
    return 0;
}
