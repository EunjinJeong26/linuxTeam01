#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "session.h"

#define SESSION_FILE "/.devlog/session"

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

    char line[MAX_USERNAME_LEN];
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    /* 개행 제거 */
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

    strncpy(out->username, line, MAX_USERNAME_LEN - 1);
    out->username[MAX_USERNAME_LEN - 1] = '\0';
    out->token[0] = '\0';

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

    fprintf(fp, "%s\n", s->username);
    fclose(fp);
    return 0;
}

void session_clear(void) {
    char path[512];
    if (session_path(path, sizeof(path)) != 0) return;
    unlink(path);
}
