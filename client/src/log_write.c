#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "log_write.h"
#include "db.h"
#include "session.h"
#include "sysinfo.h"
#include "git_scan.h"
#include "common.h"

/* UTF-8 멀티바이트 문자 수 계산 (시작 바이트만 카운트) */
static int utf8_char_count(const char *s) {
    int count = 0;
    while (*s) {
        if ((*s & 0xC0) != 0x80) count++;
        s++;
    }
    return count;
}

/* 150자 초과 시 잘라내기 */
static void truncate_to_150_chars(char *s) {
    int count = 0;
    char *p = s;
    while (*p) {
        if ((*p & 0xC0) != 0x80) {
            count++;
            if (count > 150) { *p = '\0'; break; }
        }
        p++;
    }
}

int cmd_post(void) {
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
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                team_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
    }

    if (team_id < 0) {
        fprintf(stderr, "팀에 소속되어 있지 않습니다.\n");
        db_close(db);
        return -1;
    }

    /* 시스템 정보 수집 (실패해도 계속 진행) */
    SysInfo sysinfo = { .cpu_usage = -1.0f, .mem_usage = -1.0f };
    sysinfo_collect(&sysinfo);

    /* Git 정보 수집 (실패해도 계속 진행) */
    GitInfo gitinfo;
    memset(&gitinfo, 0, sizeof(gitinfo));
    git_scan_collect(&gitinfo);

    /* 현재 시각으로 session_date, session_hour 계산 */
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char session_date[11];
    strftime(session_date, sizeof(session_date), "%Y-%m-%d", lt);
    int session_hour = lt->tm_hour;

    /* ISO8601 posted_at */
    char posted_at[32];
    strftime(posted_at, sizeof(posted_at), "%Y-%m-%dT%H:%M:%S", lt);

    /* 코멘트 입력 */
    printf("오늘의 개발 로그를 입력하세요 (최대 150자, 엔터로 완료):\n");
    char comment[MAX_COMMENT_LEN + 4] = {0};
    if (fgets(comment, sizeof(comment), stdin)) {
        /* 개행 문자 제거 */
        size_t len = strlen(comment);
        if (len > 0 && comment[len - 1] == '\n')
            comment[len - 1] = '\0';
        /* UTF-8 기준 150자 초과 시 잘라내기 */
        if (utf8_char_count(comment) > 150)
            truncate_to_150_chars(comment);
    }

    /* logs 테이블에 INSERT */
    const char *sql_insert =
        "INSERT INTO logs "
        "(username, team_id, session_date, session_hour, "
        " cpu_usage, mem_usage, git_commit_hash, git_commit_msg, comment, posted_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "INSERT 준비 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt,  1, sess.username,           -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt,  2, team_id);
    sqlite3_bind_text(stmt,  3, session_date,            -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt,  4, session_hour);
    sqlite3_bind_double(stmt,5, (double)sysinfo.cpu_usage);
    sqlite3_bind_double(stmt,6, (double)sysinfo.mem_usage);
    sqlite3_bind_text(stmt,  7, gitinfo.last_commit_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  8, gitinfo.last_commit_msg,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  9, comment,                  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, posted_at,                -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    db_close(db);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "로그 저장 실패\n");
        return -1;
    }

    /* 성공 출력 */
    printf("[로그 저장 완료]\n");
    if (sysinfo.cpu_usage < 0.0f)
        printf("CPU: N/A | MEM: N/A\n");
    else
        printf("CPU: %.1f%% | MEM: %.1f%%\n", sysinfo.cpu_usage, sysinfo.mem_usage);

    if (gitinfo.last_commit_hash[0] == '\0') {
        printf("Git: (없음)\n");
    } else {
        char short_hash[8] = {0};
        strncpy(short_hash, gitinfo.last_commit_hash, 7);
        printf("Git: %s %s\n", short_hash, gitinfo.last_commit_msg);
    }

    return 0;
}
