#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "auth.h"
#include "db.h"
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

    sqlite3 *db = db_open();
    if (!db) return -1;

    /* username 중복 확인 */
    const char *sql_check = "SELECT 1 FROM users WHERE username = ? LIMIT 1;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "cmd_register: prepare 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    if (exists) {
        fprintf(stderr, "이미 존재하는 사용자명입니다.\n");
        db_close(db);
        return -1;
    }

    /* ISO8601 형식 created_at 생성 */
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);
    char created_at[20];
    strftime(created_at, sizeof(created_at), "%Y-%m-%dT%H:%M:%S", tm_info);

    const char *sql_insert =
        "INSERT INTO users (username, password, created_at) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "cmd_register: prepare 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, username,   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password,   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, created_at, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    db_close(db);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "cmd_register: INSERT 실패\n");
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

    sqlite3 *db = db_open();
    if (!db) return -1;

    const char *sql =
        "SELECT 1 FROM users WHERE username = ? AND password = ? LIMIT 1;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "cmd_login: prepare 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int matched = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    db_close(db);

    if (!matched) {
        fprintf(stderr, "사용자명 또는 비밀번호가 올바르지 않습니다.\n");
        return -1;
    }

    Session s;
    strncpy(s.username, username, MAX_USERNAME_LEN - 1);
    s.username[MAX_USERNAME_LEN - 1] = '\0';
    s.token[0] = '\0';

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
