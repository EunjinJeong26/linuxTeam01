#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "team.h"
#include "db.h"
#include "session.h"
#include "common.h"

static void gen_invite_code(char *buf, int len) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int clen = (int)(sizeof(charset) - 1);
    for (int i = 0; i < len - 1; i++)
        buf[i] = charset[rand() % clen];
    buf[len - 1] = '\0';
}

static void iso8601_now(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *tm_info = gmtime(&t);
    strftime(buf, size, "%Y-%m-%dT%H:%M:%SZ", tm_info);
}

/* members 테이블에서 username 존재 여부: 1=소속, 0=미소속, -1=오류 */
static int is_member(sqlite3 *db, const char *username) {
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM members WHERE username = ? LIMIT 1;",
            -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    int found = (sqlite3_step(stmt) == SQLITE_ROW) ? 1 : 0;
    sqlite3_finalize(stmt);
    return found;
}

static int team_create(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: devlog team create <name>\n");
        return -1;
    }
    const char *team_name = argv[3];

    Session sess;
    if (session_load(&sess) != 0) {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    sqlite3 *db = db_open();
    if (!db) return -1;

    int mc = is_member(db, sess.username);
    if (mc < 0) { db_close(db); return -1; }
    if (mc == 1) {
        fprintf(stderr, "이미 팀에 소속되어 있습니다.\n");
        db_close(db);
        return -1;
    }

    srand((unsigned int)time(NULL));
    char invite_code[9];
    gen_invite_code(invite_code, sizeof(invite_code));

    char created_at[32];
    iso8601_now(created_at, sizeof(created_at));

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO teams (team_name, invite_code, created_at) VALUES (?, ?, ?);",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_text(stmt, 1, team_name,   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, invite_code, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, created_at,  -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) { db_close(db); return -1; }

    sqlite3_int64 team_id = sqlite3_last_insert_rowid(db);

    char joined_at[32];
    iso8601_now(joined_at, sizeof(joined_at));

    if (sqlite3_prepare_v2(db,
            "INSERT INTO members (team_id, username, role, joined_at) VALUES (?, ?, 'owner', ?);",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_int64(stmt, 1, team_id);
    sqlite3_bind_text(stmt,  2, sess.username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  3, joined_at,     -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    db_close(db);

    if (rc != SQLITE_DONE) return -1;

    printf("팀 생성 완료: %s (초대 코드: %s)\n", team_name, invite_code);
    return 0;
}

static int team_join(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: devlog team join <code>\n");
        return -1;
    }
    const char *code = argv[3];

    Session sess;
    if (session_load(&sess) != 0) {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    sqlite3 *db = db_open();
    if (!db) return -1;

    int mc = is_member(db, sess.username);
    if (mc < 0) { db_close(db); return -1; }
    if (mc == 1) {
        fprintf(stderr, "이미 팀에 소속되어 있습니다.\n");
        db_close(db);
        return -1;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db,
            "SELECT team_id, team_name FROM teams WHERE invite_code = ? LIMIT 1;",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_text(stmt, 1, code, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "유효하지 않은 초대 코드입니다.\n");
        db_close(db);
        return -1;
    }

    sqlite3_int64 team_id = sqlite3_column_int64(stmt, 0);
    char team_name[MAX_TEAM_NAME_LEN];
    strncpy(team_name, (const char *)sqlite3_column_text(stmt, 1), MAX_TEAM_NAME_LEN - 1);
    team_name[MAX_TEAM_NAME_LEN - 1] = '\0';
    sqlite3_finalize(stmt);

    char joined_at[32];
    iso8601_now(joined_at, sizeof(joined_at));

    if (sqlite3_prepare_v2(db,
            "INSERT INTO members (team_id, username, role, joined_at) VALUES (?, ?, 'member', ?);",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_int64(stmt, 1, team_id);
    sqlite3_bind_text(stmt,  2, sess.username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  3, joined_at,     -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    db_close(db);

    if (rc != SQLITE_DONE) return -1;

    printf("팀 참가 완료: %s\n", team_name);
    return 0;
}

static int team_info(void) {
    Session sess;
    if (session_load(&sess) != 0) {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    sqlite3 *db = db_open();
    if (!db) return -1;

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db,
            "SELECT t.team_name, t.invite_code FROM members m "
            "JOIN teams t ON m.team_id = t.team_id "
            "WHERE m.username = ? LIMIT 1;",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_text(stmt, 1, sess.username, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "소속된 팀이 없습니다.\n");
        db_close(db);
        return -1;
    }

    char team_name[MAX_TEAM_NAME_LEN];
    char invite_code[MAX_INVITE_CODE_LEN];
    strncpy(team_name,   (const char *)sqlite3_column_text(stmt, 0), MAX_TEAM_NAME_LEN - 1);
    strncpy(invite_code, (const char *)sqlite3_column_text(stmt, 1), MAX_INVITE_CODE_LEN - 1);
    team_name[MAX_TEAM_NAME_LEN - 1]     = '\0';
    invite_code[MAX_INVITE_CODE_LEN - 1] = '\0';
    sqlite3_finalize(stmt);

    printf("팀 이름 : %s\n", team_name);
    printf("초대 코드: %s\n", invite_code);
    printf("멤버 목록:\n");

    if (sqlite3_prepare_v2(db,
            "SELECT m.username, m.role FROM members m "
            "JOIN teams t ON m.team_id = t.team_id "
            "WHERE t.team_name = ? ORDER BY m.joined_at ASC;",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_text(stmt, 1, team_name, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *uname = (const char *)sqlite3_column_text(stmt, 0);
        const char *role  = (const char *)sqlite3_column_text(stmt, 1);
        printf("  - %s (%s)\n", uname, role);
    }
    sqlite3_finalize(stmt);
    db_close(db);
    return 0;
}

static int team_leave(void) {
    Session sess;
    if (session_load(&sess) != 0) {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    sqlite3 *db = db_open();
    if (!db) return -1;

    int mc = is_member(db, sess.username);
    if (mc < 0) { db_close(db); return -1; }
    if (mc == 0) {
        fprintf(stderr, "소속된 팀이 없습니다.\n");
        db_close(db);
        return -1;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db,
            "DELETE FROM members WHERE username = ?;",
            -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db); return -1;
    }
    sqlite3_bind_text(stmt, 1, sess.username, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    db_close(db);

    if (rc != SQLITE_DONE) return -1;

    printf("팀 탈퇴 완료\n");
    return 0;
}

int cmd_team(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog team <create|join|info|leave> ...\n");
        return 1;
    }
    const char *sub = argv[2];
    if (strcmp(sub, "create") == 0) return team_create(argc, argv);
    if (strcmp(sub, "join")   == 0) return team_join(argc, argv);
    if (strcmp(sub, "info")   == 0) return team_info();
    if (strcmp(sub, "leave")  == 0) return team_leave();

    fprintf(stderr, "Usage: devlog team <create|join|info|leave> ...\n");
    return 1;
}
