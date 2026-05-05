/* TODO: 김규린 담당 구현 */

#include "diary_read.h"
#include "fileio.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_DIARIES 256   /* 한 번에 읽을 수 있는 최대 일기 수 */

/* 일기 파일 하나를 파싱 → Diary 구조체에 채움 */
static int parse_diary_file(const char *filepath, Diary *out) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) return RET_FAIL;

    char line[MAX_CONTENT_LEN];
    memset(out, 0, sizeof(Diary));

    while (fgets(line, MAX_CONTENT_LEN, fp)) {
        line[strcspn(line, "\n")] = '\0';   /* \n 제거 */

        if      (strncmp(line, "id=",         3)  == 0) strncpy(out->id,         line + 3,  sizeof(out->id)         - 1);
        else if (strncmp(line, "author=",     7)  == 0) strncpy(out->author,     line + 7,  sizeof(out->author)     - 1);
        else if (strncmp(line, "title=",      6)  == 0) strncpy(out->title,      line + 6,  sizeof(out->title)      - 1);
        else if (strncmp(line, "visibility=", 11) == 0) strncpy(out->visibility, line + 11, sizeof(out->visibility) - 1);
        else if (strncmp(line, "createdAt=",  10) == 0) strncpy(out->createdAt,  line + 10, sizeof(out->createdAt)  - 1);
        else if (strncmp(line, "updatedAt=",  10) == 0) strncpy(out->updatedAt,  line + 10, sizeof(out->updatedAt)  - 1);
        else if (strncmp(line, "content=",    8)  == 0) strncpy(out->content,    line + 8,  sizeof(out->content)    - 1);
    }

    fclose(fp);
    return RET_OK;
}

/* 목록 한 줄 출력 */
static void print_diary_row(int idx, const Diary *d) {
    char date[11] = {0};
    strncpy(date, d->createdAt, 10);
    printf("[%3d] %-10s | %-15s | %-20s | %s\n",
           idx, date, d->author, d->id, d->title);
}

/* 일기 본문 출력 */
static void print_diary_detail(const Diary *d) {
    printf("\n");
    printf("========================================\n");
    printf("제목    : %s\n", d->title);
    printf("작성자  : %s\n", d->author);
    printf("공개여부: %s\n", d->visibility);
    printf("작성일  : %s\n", d->createdAt);
    printf("수정일  : %s\n", d->updatedAt);
    printf("----------------------------------------\n");
    printf("%s\n", d->content);
    printf("========================================\n\n");
}

/* diary read — 내 일기 목록 출력 + 번호 선택으로 상세 조회 */
int list_my_diaries(const User *user) {
    if (!user) return RET_FAIL;

    char dirpath[MAX_PATH_LEN];
    build_diary_dir_path(user->username, dirpath, sizeof(dirpath));

    DIR *dir = opendir(dirpath);
    if (!dir) {
        printf("아직 작성한 일기가 없습니다.\n");
        return RET_OK;
    }

    /* 일기 파일 전부 읽어서 배열에 저장 */
    Diary diaries[MAX_DIARIES];
    int count = 0;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && count < MAX_DIARIES) {
        size_t len = strlen(ent->d_name);
        if (len < 5 || strcmp(ent->d_name + len - 4, ".txt") != 0) continue;

        char filepath[MAX_PATH_LEN];
        build_diary_file_path(user->username, ent->d_name,
                              filepath, sizeof(filepath));

        if (parse_diary_file(filepath, &diaries[count]) == RET_OK)
            count++;
    }
    closedir(dir);

    /* 목록 출력 */
    printf("\n=== 내 일기 목록 (%s) ===\n", user->nickname);
    printf("[번호] %-10s | %-10s | %-20s | %s\n", "날짜", "공개여부", "ID", "제목");
    printf("---------------------------------------------------------------\n");

    if (count == 0) {
        printf("(작성한 일기가 없습니다)\n");
        return RET_OK;
    }

    for (int i = 0; i < count; i++)
        print_diary_row(i + 1, &diaries[i]);

    printf("\n총 %d편\n", count);

    /* 번호 입력받아서 상세 조회 */
    printf("\n읽을 일기 번호 입력 (0=종료): ");
    int choice;
    if (scanf("%d", &choice) != 1) return RET_OK;

    if (choice == 0) return RET_OK;

    if (choice < 1 || choice > count) {
        fprintf(stderr, "잘못된 번호입니다.\n");
        return RET_FAIL;
    }

    print_diary_detail(&diaries[choice - 1]);
    return RET_OK;
}

/* 
diary public — 전체 공개 일기 목록 + 번호 선택으로 상세 조회
DIARIES_DIR 아래 모든 사용자 폴더 순회,
visibility=PUBLIC 인 일기만 출력
*/
int list_public_diaries(void) {
    DIR *base = opendir(DIARIES_DIR);
    if (!base) {
        printf("아직 공개된 일기가 없습니다.\n");
        return RET_OK;
    }

    /* PUBLIC 일기 전부 읽어서 배열에 저장 */
    Diary diaries[MAX_DIARIES];
    int count = 0;

    struct dirent *user_ent;
    while ((user_ent = readdir(base)) != NULL) {
        if (user_ent->d_name[0] == '.') continue;

        char user_dir[MAX_PATH_LEN];
        build_diary_dir_path(user_ent->d_name, user_dir, sizeof(user_dir));
        if (!dir_exists(user_dir)) continue;

        DIR *udir = opendir(user_dir);
        if (!udir) continue;

        struct dirent *ent;
        while ((ent = readdir(udir)) != NULL && count < MAX_DIARIES) {
            size_t len = strlen(ent->d_name);
            if (len < 5 || strcmp(ent->d_name + len - 4, ".txt") != 0) continue;

            char filepath[MAX_PATH_LEN];
            build_diary_file_path(user_ent->d_name, ent->d_name,
                                  filepath, sizeof(filepath));

            if (parse_diary_file(filepath, &diaries[count]) != RET_OK) continue;
            if (strcmp(diaries[count].visibility, VISIBILITY_PUBLIC) != 0) continue;
            count++;
        }
        closedir(udir);
    }
    closedir(base);

    /* 목록 출력 */
    printf("\n=== 공개 일기 목록 ===\n");
    printf("[번호] %-10s | %-15s | %-20s | %s\n", "날짜", "작성자", "ID", "제목");
    printf("---------------------------------------------------------------\n");

    if (count == 0) {
        printf("(공개된 일기가 없습니다)\n");
        return RET_OK;
    }

    for (int i = 0; i < count; i++)
        print_diary_row(i + 1, &diaries[i]);

    printf("\n총 %d편\n", count);

    /* 번호 입력받아 상세 조회 */
    printf("\n읽을 일기 번호 입력 (0=종료): ");
    int choice;
    if (scanf("%d", &choice) != 1) return RET_OK;

    if (choice == 0) return RET_OK;

    if (choice < 1 || choice > count) {
        fprintf(stderr, "잘못된 번호입니다.\n");
        return RET_FAIL;
    }

    print_diary_detail(&diaries[choice - 1]);
    return RET_OK;
}

/* 
read_diary — 일기 상세 조회
권한 규칙:
  PUBLIC  → 누구나 열람 가능 (viewer == NULL 도 OK)
  PRIVATE → 작성자 본인(viewer->username == author)만 열람 가능
*/
int read_diary(const char *username, const char *filename, const User *viewer) {
    if (!username || !filename) return RET_FAIL;

    char filepath[MAX_PATH_LEN];
    build_diary_file_path(username, filename, filepath, sizeof(filepath));

    Diary d;
    if (parse_diary_file(filepath, &d) != RET_OK) {
        fprintf(stderr, "일기를 찾을 수 없습니다: %s/%s\n", username, filename);
        return RET_FAIL;
    }

    /* 권한 체크 */
    int is_owner = viewer && (strcmp(viewer->username, d.author) == 0);
    if (strcmp(d.visibility, VISIBILITY_PRIVATE) == 0 && !is_owner) {
        fprintf(stderr, "비공개 일기입니다. 열람 권한이 없습니다.\n");
        return RET_FAIL;
    }

    print_diary_detail(&d);
    return RET_OK;
}