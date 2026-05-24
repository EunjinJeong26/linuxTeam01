#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "db.h"

#define DB_FILENAME "/.devlog/cache.db"
#define DEVLOG_DIR  "/.devlog"

static const char *DDL_USERS =
    "CREATE TABLE IF NOT EXISTS users ("
    "    username   TEXT PRIMARY KEY,"
    "    password   TEXT NOT NULL,"
    "    created_at TEXT NOT NULL"
    ");";

static const char *DDL_GIT_DIRS =
    "CREATE TABLE IF NOT EXISTS git_dirs ("
    "    path     TEXT PRIMARY KEY,"
    "    added_at TEXT NOT NULL"
    ");";

static const char *DDL_TEAMS =
    "CREATE TABLE IF NOT EXISTS teams ("
    "    team_id     INTEGER PRIMARY KEY,"
    "    team_name   TEXT NOT NULL,"
    "    invite_code TEXT NOT NULL,"
    "    created_at  TEXT NOT NULL"
    ");";

static const char *DDL_MEMBERS =
    "CREATE TABLE IF NOT EXISTS members ("
    "    team_id   INTEGER NOT NULL,"
    "    username  TEXT    NOT NULL,"
    "    role      TEXT    NOT NULL,"
    "    joined_at TEXT    NOT NULL,"
    "    PRIMARY KEY (team_id, username)"
    ");";

static const char *DDL_LOGS =
    "CREATE TABLE IF NOT EXISTS logs ("
    "    log_id          INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    username        TEXT    NOT NULL,"
    "    team_id         INTEGER NOT NULL,"
    "    session_date    TEXT    NOT NULL,"
    "    session_hour    INTEGER NOT NULL,"
    "    cpu_usage       REAL,"
    "    mem_usage       REAL,"
    "    git_commit_hash TEXT,"
    "    git_commit_msg  TEXT,"
    "    comment         TEXT,"
    "    posted_at       TEXT    NOT NULL,"
    "    UNIQUE(username, team_id, session_date, session_hour)"
    ");";

sqlite3 *db_open(void) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "db_open: HOME 환경변수가 설정되지 않음\n");
        return NULL;
    }

    /* ~/.devlog/ 디렉터리가 없으면 생성 */
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s%s", home, DEVLOG_DIR);
    struct stat st;
    if (stat(dir_path, &st) != 0) {
        if (mkdir(dir_path, 0755) != 0) {
            fprintf(stderr, "db_open: 디렉터리 생성 실패: %s\n", dir_path);
            return NULL;
        }
    }

    char db_path[512];
    snprintf(db_path, sizeof(db_path), "%s%s", home, DB_FILENAME);

    sqlite3 *db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "db_open: DB 연결 실패: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return db;
}

void db_close(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

int db_init(sqlite3 *db) {
    const char *tables[] = {
        DDL_USERS,
        DDL_GIT_DIRS,
        DDL_TEAMS,
        DDL_MEMBERS,
        DDL_LOGS,
        NULL
    };

    char *errmsg = NULL;
    for (int i = 0; tables[i] != NULL; i++) {
        if (sqlite3_exec(db, tables[i], NULL, NULL, &errmsg) != SQLITE_OK) {
            fprintf(stderr, "db_init: 테이블 생성 실패: %s\n", errmsg);
            sqlite3_free(errmsg);
            return -1;
        }
    }

    return 0;
}
