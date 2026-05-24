#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "git_scan.h"
#include "db.h"

#define MAX_DEPTH 5

/* .git/logs/HEAD 마지막 줄 파싱: hash, message, timestamp 추출 */
static int parse_head_log(const char *git_dir, char *hash, char *msg, long long *ts) {
    char path[600];
    snprintf(path, sizeof(path), "%s/logs/HEAD", git_dir);

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char line[1024];
    char last[1024];
    last[0] = '\0';
    while (fgets(line, sizeof(line), fp))
        strncpy(last, line, sizeof(last) - 1);
    fclose(fp);

    if (last[0] == '\0') return -1;

    /* 형식: <old> <new> <author info> <timestamp> <tz>\t<message> */
    char old_hash[41], new_hash[41], rest[1024];
    if (sscanf(last, "%40s %40s %[^\n]", old_hash, new_hash, rest) < 3) return -1;

    /* rest에서 마지막 두 공백-구분 필드(timestamp tz) 앞까지 author, timestamp 추출 */
    /* timestamp는 \t 이전 마지막 숫자 필드 */
    char *tab = strchr(rest, '\t');
    if (tab) {
        strncpy(msg, tab + 1, 255);
        msg[255] = '\0';
        /* 개행 제거 */
        size_t mlen = strlen(msg);
        if (mlen > 0 && msg[mlen-1] == '\n') msg[mlen-1] = '\0';
    } else {
        msg[0] = '\0';
    }

    /* timestamp: \t 이전 텍스트에서 마지막에서 두 번째 토큰 */
    char before_tab[1024];
    size_t before_len = tab ? (size_t)(tab - rest) : strlen(rest);
    strncpy(before_tab, rest, before_len);
    before_tab[before_len] = '\0';

    /* 뒤에서부터 두 토큰(tz, timestamp) 파싱 */
    char tz_tok[16];
    long long ts_val = 0;
    if (sscanf(before_tab + (before_len > 20 ? before_len - 20 : 0),
               "%lld %15s", &ts_val, tz_tok) >= 1)
        *ts = ts_val;
    else
        *ts = 0;

    strncpy(hash, new_hash, 40);
    hash[40] = '\0';
    return 0;
}

/* depth 제한 재귀 탐색으로 .git 디렉터리를 찾아 후보 목록에 추가 */
typedef struct {
    char path[512];    /* .git 디렉터리 경로 */
    char repo[512];    /* 저장소 루트 경로 */
    long long ts;
    char hash[41];
    char msg[256];
} GitCandidate;

static void scan_recursive(const char *base, int depth,
                            GitCandidate *best) {
    if (depth > MAX_DEPTH) return;

    DIR *dir = opendir(base);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        /* . .. 및 .git 외 숨김 디렉터리 건너뜀 */
        if (ent->d_name[0] == '.') {
            if (strcmp(ent->d_name, ".git") != 0) continue;
        }

        char full[600];
        snprintf(full, sizeof(full), "%s/%s", base, ent->d_name);

        struct stat st;
        if (stat(full, &st) != 0) continue;
        if (!S_ISDIR(st.st_mode)) continue;

        if (strcmp(ent->d_name, ".git") == 0) {
            char hash[41], msg[256];
            long long ts = 0;
            if (parse_head_log(full, hash, msg, &ts) == 0) {
                if (ts > best->ts) {
                    best->ts = ts;
                    strncpy(best->path, full, 511);
                    strncpy(best->repo, base, 511);
                    strncpy(best->hash, hash, 40);
                    strncpy(best->msg,  msg,  255);
                }
            }
            /* .git 내부는 탐색하지 않음 */
            continue;
        }

        scan_recursive(full, depth + 1, best);
    }
    closedir(dir);
}

/* ── cmd_git 서브커맨드 ── */

static int git_add(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "존재하지 않는 디렉터리입니다: %s\n", path);
        return -1;
    }

    sqlite3 *db = db_open();
    if (!db) return -1;

    const char *sql =
        "INSERT OR IGNORE INTO git_dirs (path, added_at)"
        " VALUES (?, strftime('%Y-%m-%dT%H:%M:%S', 'now'));";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "git_add: prepare 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    db_close(db);

    printf("등록 완료: %s\n", path);
    return 0;
}

static int git_remove(const char *path) {
    sqlite3 *db = db_open();
    if (!db) return -1;

    const char *sql = "DELETE FROM git_dirs WHERE path = ?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "git_remove: prepare 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) {
        fprintf(stderr, "등록되지 않은 경로입니다: %s\n", path);
        db_close(db);
        return -1;
    }
    db_close(db);

    printf("등록 해제 완료: %s\n", path);
    return 0;
}

static int git_list(void) {
    sqlite3 *db = db_open();
    if (!db) return -1;

    const char *sql = "SELECT path FROM git_dirs ORDER BY added_at ASC;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "git_list: prepare 실패: %s\n", sqlite3_errmsg(db));
        db_close(db);
        return -1;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%s\n", (const char *)sqlite3_column_text(stmt, 0));
        count++;
    }
    sqlite3_finalize(stmt);
    db_close(db);

    if (count == 0)
        printf("등록된 디렉터리가 없습니다.\n");
    return 0;
}

int cmd_git(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog git <add|remove|list> [path]\n");
        return 1;
    }
    const char *sub = argv[2];
    if (strcmp(sub, "add") == 0) {
        if (argc < 4) { fprintf(stderr, "Usage: devlog git add <path>\n"); return 1; }
        return git_add(argv[3]);
    } else if (strcmp(sub, "remove") == 0) {
        if (argc < 4) { fprintf(stderr, "Usage: devlog git remove <path>\n"); return 1; }
        return git_remove(argv[3]);
    } else if (strcmp(sub, "list") == 0) {
        return git_list();
    } else {
        fprintf(stderr, "Usage: devlog git <add|remove|list> [path]\n");
        return 1;
    }
}

int git_scan_collect(GitInfo *out) {
    out->repo_path[0]        = '\0';
    out->last_commit_hash[0] = '\0';
    out->last_commit_msg[0]  = '\0';

    sqlite3 *db = db_open();
    if (!db) return -1;

    const char *sql = "SELECT path FROM git_dirs;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        db_close(db);
        return -1;
    }

    GitCandidate best;
    memset(&best, 0, sizeof(best));
    best.ts = -1;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *base = (const char *)sqlite3_column_text(stmt, 0);
        scan_recursive(base, 0, &best);
    }
    sqlite3_finalize(stmt);
    db_close(db);

    if (best.ts < 0) return -1;

    strncpy(out->repo_path,        best.repo, sizeof(out->repo_path) - 1);
    strncpy(out->last_commit_hash, best.hash, sizeof(out->last_commit_hash) - 1);
    strncpy(out->last_commit_msg,  best.msg,  sizeof(out->last_commit_msg) - 1);
    return 0;
}
