#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "log_read.h"
#include "db.h"
#include "session.h"
#include "common.h"

int cmd_show(int argc, char *argv[]) {
    Session sess;
    if (session_load(&sess) != 0) {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    sqlite3 *db = db_open();
    if (!db) return -1;

    /* members 테이블에서 team_id 조회 */
    int team_id = -1;
    {
        const char *sql = "SELECT team_id FROM members WHERE username = ? LIMIT 1;";
        sqlite3_stmt *stmt = NULL;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, sess.username, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW)
                team_id = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
        }
    }

    if (team_id < 0) {
        fprintf(stderr, "팀에 소속되어 있지 않습니다.\n");
        db_close(db);
        return -1;
    }

    /* 플래그 파싱 */
    int flag_my   = 0;
    char date_buf[11] = {0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--my") == 0) {
            flag_my = 1;
        } else if (strcmp(argv[i], "--date") == 0 && i + 1 < argc) {
            strncpy(date_buf, argv[i + 1], 10);
            date_buf[10] = '\0';
            i++;
        }
    }

    /* --date 미지정 시 오늘 날짜 사용 */
    if (date_buf[0] == '\0') {
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", lt);
    }

    /* 쿼리 구성 */
    const char *sql_my  =
        "SELECT session_hour, username, cpu_usage, mem_usage, "
        "       git_commit_hash, git_commit_msg, comment "
        "FROM logs "
        "WHERE username = ? AND team_id = ? AND session_date = ? "
        "ORDER BY session_hour ASC, username ASC;";

    const char *sql_all =
        "SELECT session_hour, username, cpu_usage, mem_usage, "
        "       git_commit_hash, git_commit_msg, comment "
        "FROM logs "
        "WHERE team_id = ? AND session_date = ? "
        "ORDER BY session_hour ASC, username ASC;";

    sqlite3_stmt *stmt = NULL;
    int rc;

    if (flag_my) {
        rc = sqlite3_prepare_v2(db, sql_my, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, sess.username, -1, SQLITE_STATIC);
            sqlite3_bind_int (stmt, 2, team_id);
            sqlite3_bind_text(stmt, 3, date_buf,      -1, SQLITE_STATIC);
        }
    } else {
        rc = sqlite3_prepare_v2(db, sql_all, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            sqlite3_bind_int (stmt, 1, team_id);
            sqlite3_bind_text(stmt, 2, date_buf, -1, SQLITE_STATIC);
        }
    }

    if (rc != SQLITE_OK) {
        fprintf(stderr, "쿼리 준비 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }

    /* 결과 출력 */
    int found = 0;
    printf("=== %s 팀 현황 ===\n", date_buf);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!found) printf("\n");
        found = 1;

        int   hour      = sqlite3_column_int (stmt, 0);
        const char *uname = (const char *)sqlite3_column_text(stmt, 1);
        double cpu      = sqlite3_column_double(stmt, 2);
        double mem      = sqlite3_column_double(stmt, 3);
        const char *ghash = (const char *)sqlite3_column_text(stmt, 4);
        const char *gmsg  = (const char *)sqlite3_column_text(stmt, 5);
        const char *cmt   = (const char *)sqlite3_column_text(stmt, 6);

        printf("[%02d시] %s\n", hour, uname ? uname : "");

        /* CPU/MEM: 음수면 N/A */
        if (cpu < 0.0 || mem < 0.0)
            printf("  CPU: N/A | MEM: N/A\n");
        else
            printf("  CPU: %.1f%% | MEM: %.1f%%\n", cpu, mem);

        /* Git */
        if (!ghash || ghash[0] == '\0') {
            printf("  Git: (없음)\n");
        } else {
            char short_hash[8] = {0};
            strncpy(short_hash, ghash, 7);
            printf("  Git: %s %s\n", short_hash, gmsg ? gmsg : "");
        }

        /* 코멘트 */
        if (!cmt || cmt[0] == '\0')
            printf("  (코멘트 없음)\n");
        else
            printf("  %s\n", cmt);
    }

    sqlite3_finalize(stmt);
    db_close(db);

    if (!found)
        printf("해당 날짜의 로그가 없습니다.\n");

    return 0;
}
