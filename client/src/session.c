#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "session.h"
#include "common.h"

#define SESSION_FILE "/.devlog/session"

/* fgets로 읽은 줄의 맨 끝 개행 제거 */
static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
}

static int session_path(char *buf, size_t size) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "session: HOME 환경변수가 설정되지 않음\n");
        return -1;
    }
    snprintf(buf, size, "%s%s", home, SESSION_FILE);
    return 0;
}

int session_load(Session *out) {
    char path[512];
    if (session_path(path, sizeof(path)) != 0) return -1;

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    /* 1줄: username (필수) */
    char username[MAX_USERNAME_LEN];
    if (fgets(username, sizeof(username), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    strip_newline(username);

    strncpy(out->username, username, MAX_USERNAME_LEN - 1);
    out->username[MAX_USERNAME_LEN - 1] = '\0';

    /* 2줄: token (없는 구버전 세션 파일은 빈 문자열로 처리) */
    char token[MAX_TOKEN_LEN];
    if (fgets(token, sizeof(token), fp) != NULL) {
        strip_newline(token);
        strncpy(out->token, token, MAX_TOKEN_LEN - 1);
        out->token[MAX_TOKEN_LEN - 1] = '\0';
    } else {
        out->token[0] = '\0';
    }

    fclose(fp);
    return 0;
}

int session_save(const Session *s) {
    char path[512];
    if (session_path(path, sizeof(path)) != 0) return -1;

    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "session_save: 파일 열기 실패: %s\n", path);
        return -1;
    }

    fprintf(fp, "%s\n%s\n", s->username, s->token);
    fclose(fp);
    return 0;
}

void session_clear(void) {
    char path[512];
    if (session_path(path, sizeof(path)) != 0) return;
    unlink(path);
}
