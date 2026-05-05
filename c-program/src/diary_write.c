#include "diary_write.h"
#include "common.h"
#include "fileio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

/*
 * 해당 사용자의 일기 폴더에서 오늘 날짜로 저장된 파일이 몇 개인지 세어
 * 다음 시퀀스 번호를 반환한다.
 * 예) 오늘 이미 001, 002 가 있으면 3 반환.
 */
static int next_seq_for_today(const char *dir, const char *date_prefix)
{
    DIR *dp = opendir(dir);
    if (!dp) return 1;

    int max_seq = 0;
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        /* 파일명 형식: YYYY-MM-DD_NNN.txt */
        if (strncmp(entry->d_name, date_prefix, strlen(date_prefix)) == 0) {
            /* "_NNN.txt" 부분에서 숫자 추출 */
            const char *underscore = strchr(entry->d_name, '_');
            if (underscore) {
                int seq = atoi(underscore + 1);
                if (seq > max_seq) max_seq = seq;
            }
        }
    }
    closedir(dp);
    return max_seq + 1;
}

/* ------------------------------------------------------------------ */
/*  generate_diary_filename                                             */
/* ------------------------------------------------------------------ */

/*
 * 일기 파일명을 생성한다.
 * 형식: YYYY-MM-DD_NNN.txt
 * 같은 날 여러 개 작성해도 NNN이 증가하며 충돌하지 않는다.
 *
 * @param username  작성자 username
 * @param out       결과 문자열을 저장할 버퍼
 * @param out_size  버퍼 크기
 */
void generate_diary_filename(const char *username, char *out, int out_size)
{
    /* 오늘 날짜 구하기 */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char date_prefix[16];
    strftime(date_prefix, sizeof(date_prefix), "%Y-%m-%d", t);

    /* 사용자 일기 폴더 경로 */
    char dir[MAX_PATH_LEN];
    build_diary_dir_path(username, dir, sizeof(dir));

    int seq = next_seq_for_today(dir, date_prefix);

    /* 최종 파일명: YYYY-MM-DD_NNN.txt */
    snprintf(out, out_size, "%s_%03d.txt", date_prefix, seq);
}

/* ------------------------------------------------------------------ */
/*  save_diary                                                          */
/* ------------------------------------------------------------------ */

/*
 * Diary 구조체를 파일에 저장한다.
 * 저장 경로: data/diaries/{username}/{filename}
 *
 * @param diary     저장할 일기 구조체
 * @param username  작성자 username
 * @return RET_OK / RET_FAIL
 */
int save_diary(const Diary *diary, const char *username)
{
    char dir[MAX_PATH_LEN];
    build_diary_dir_path(username, dir, sizeof(dir));

    char filepath[MAX_PATH_LEN];
    build_diary_file_path(username, diary->filename, filepath, sizeof(filepath));

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        perror("일기 파일 생성 실패");
        return RET_FAIL;
    }

    const char *vis_str = (strcmp(diary->visibility, VISIBILITY_PUBLIC) == 0)
                             ? VISIBILITY_PUBLIC
                             : VISIBILITY_PRIVATE;

    fprintf(fp, "id=%s\n",          diary->id);
    fprintf(fp, "author=%s\n",      username);
    fprintf(fp, "title=%s\n",       diary->title);
    fprintf(fp, "visibility=%s\n",  vis_str);
    fprintf(fp, "createdAt=%s\n",   diary->createdAt);
    fprintf(fp, "updatedAt=%s\n",   diary->updatedAt);
    fprintf(fp, "content=%s\n",     diary->content);

    fclose(fp);
    return RET_OK;
}

/* ------------------------------------------------------------------ */
/*  write_diary  (메인 커맨드 핸들러)                                   */
/* ------------------------------------------------------------------ */

/*
 * `diary write` 커맨드 핸들러.
 * 1. 사용자 일기 폴더 확인/생성
 * 2. 제목 · 내용 · 공개여부 입력
 * 3. Diary 구조체 구성
 * 4. save_diary()로 파일 저장
 *
 * @param author  현재 로그인된 사용자 (session에서 읽어온 값)
 * @return RET_OK / RET_FAIL
 */
int write_diary(const User *author)
{
    /* 1. 사용자 일기 폴더 확인/생성 */
    char dir[MAX_PATH_LEN];
    build_diary_dir_path(author->username, dir, sizeof(dir));
    make_dir(dir);

    /* 2. 입력 */
    Diary diary;
    memset(&diary, 0, sizeof(diary));

    printf("제목: ");
    if (!fgets(diary.title, sizeof(diary.title), stdin)) return RET_FAIL;
    /* 개행 제거 */
    diary.title[strcspn(diary.title, "\n")] = '\0';

    if (strlen(diary.title) == 0) {
        printf("제목을 입력해야 합니다.\n");
        return RET_FAIL;
    }

    printf("내용 (한 줄): ");
    if (!fgets(diary.content, sizeof(diary.content), stdin)) return RET_FAIL;
    diary.content[strcspn(diary.content, "\n")] = '\0';

    /* 공개 여부 */
    char vis_input[8];
    printf("공개 여부 (1: 공개 / 2: 비공개): ");
    if (!fgets(vis_input, sizeof(vis_input), stdin)) return RET_FAIL;
    vis_input[strcspn(vis_input, "\n")] = '\0';

    if (strcmp(vis_input, "1") == 0) {
        strncpy(diary.visibility, VISIBILITY_PUBLIC, sizeof(diary.visibility) - 1);
        diary.visibility[sizeof(diary.visibility) - 1] = '\0';
    } else {
        strncpy(diary.visibility, VISIBILITY_PRIVATE, sizeof(diary.visibility) - 1);
        diary.visibility[sizeof(diary.visibility) - 1] = '\0';
    }

    /* 3. 파일명 · 시간 · id 채우기 */
    generate_diary_filename(author->username, diary.filename, sizeof(diary.filename));

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", t);

    /* id = 날짜_NNN  (파일명에서 .txt 제거) */
    strncpy(diary.id, diary.filename, sizeof(diary.id) - 1);
    char *dot = strrchr(diary.id, '.');
    if (dot) *dot = '\0';

    strncpy(diary.createdAt, timestamp, sizeof(diary.createdAt) - 1);
    strncpy(diary.updatedAt, timestamp, sizeof(diary.updatedAt) - 1);

    /* 4. 저장 */
    if (save_diary(&diary, author->username) != RET_OK) {
        printf("일기 저장에 실패했습니다.\n");
        return RET_FAIL;
    }

    printf("일기가 저장되었습니다: %s\n", diary.filename);
    return RET_OK;
}