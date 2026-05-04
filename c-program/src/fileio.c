/**
 * fileio.c - 파일/폴더 초기화 및 경로 유틸리티 구현
 *
 * 데이터 저장 경로:
 *   기본값: ~/diary/  (홈 디렉터리 아래 diary 폴더)
 *   변경 시: export DIARY_DATA_DIR=/원하는/경로
 */

#include "fileio.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int dir_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int make_dir(const char *path) {
    if (dir_exists(path)) return RET_OK;
    if (mkdir(path, 0755) == 0) return RET_OK;
    perror(path);
    return RET_FAIL;
}

int init_data_dir(void) {
    char diary_home[MAX_PATH_LEN];

    const char *env = getenv("DIARY_DATA_DIR");
    if (env) {
        snprintf(diary_home, sizeof(diary_home), "%s", env);
    } else {
        const char *home = getenv("HOME");
        if (!home) { fprintf(stderr, "HOME 환경변수가 없습니다.\n"); return RET_FAIL; }
        snprintf(diary_home, sizeof(diary_home), "%s/diary", home);
    }

    if (make_dir(diary_home) == RET_FAIL) return RET_FAIL;

    if (chdir(diary_home) != 0) { perror("chdir"); return RET_FAIL; }

    if (make_dir(DATA_DIR) == RET_FAIL) return RET_FAIL;
    if (make_dir(DIARIES_DIR) == RET_FAIL) return RET_FAIL;

    FILE *fp = fopen(USERS_FILE, "a");
    if (!fp) { perror(USERS_FILE); return RET_FAIL; }
    fclose(fp);

    return RET_OK;
}

void build_diary_dir_path(const char *username, char *out, int out_size) {
    snprintf(out, out_size, "%s/%s", DIARIES_DIR, username);
}

void build_diary_file_path(const char *username, const char *filename,
                            char *out, int out_size) {
    snprintf(out, out_size, "%s/%s/%s", DIARIES_DIR, username, filename);
}
