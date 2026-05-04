/**
 * session.c - 로그인 세션 관리 구현
 */

#include "session.h"
#include <sys/stat.h>
#include <errno.h>

void get_session_path(char *out, int size) {
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(out, size, "%s/.diary_session", home);
}

int save_session(const User *user) {
    char path[MAX_PATH_LEN];
    get_session_path(path, sizeof(path));

    FILE *fp = fopen(path, "w");
    if (!fp) { perror(path); return RET_FAIL; }

    fprintf(fp, "id=%d\n",       user->id);
    fprintf(fp, "username=%s\n", user->username);
    fprintf(fp, "nickname=%s\n", user->nickname);

    fclose(fp);
    chmod(path, 0600);
    return RET_OK;
}

int load_session(User *out_user) {
    char path[MAX_PATH_LEN];
    get_session_path(path, sizeof(path));

    FILE *fp = fopen(path, "r");
    if (!fp) return RET_FAIL;

    memset(out_user, 0, sizeof(User));

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "id=", 3) == 0)
            out_user->id = atoi(line + 3);
        else if (strncmp(line, "username=", 9) == 0) {
            strncpy(out_user->username, line + 9, MAX_USERNAME_LEN - 1);
            out_user->username[MAX_USERNAME_LEN - 1] = '\0';
        } else if (strncmp(line, "nickname=", 9) == 0) {
            strncpy(out_user->nickname, line + 9, MAX_NICKNAME_LEN - 1);
            out_user->nickname[MAX_NICKNAME_LEN - 1] = '\0';
        }
    }

    fclose(fp);
    return (out_user->username[0] != '\0') ? RET_OK : RET_FAIL;
}

int clear_session(void) {
    char path[MAX_PATH_LEN];
    get_session_path(path, sizeof(path));

    if (remove(path) == 0 || errno == ENOENT) return RET_OK;
    perror(path);
    return RET_FAIL;
}

int is_logged_in(void) {
    char path[MAX_PATH_LEN];
    get_session_path(path, sizeof(path));

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}
