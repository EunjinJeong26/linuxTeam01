/**
 * common.h - 공통 상수, 구조체, 반환 코드 정의
 *
 * 모든 .c 파일에서 #include "common.h" 로 포함해서 사용.
 * 팀원 전체가 공유하는 타입/상수는 반드시 여기에 정의.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 문자열 길이 상수 */
#define MAX_ID_LEN       32
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 64
#define MAX_NICKNAME_LEN 32
#define MAX_TITLE_LEN    128
#define MAX_CONTENT_LEN  4096
#define MAX_DATE_LEN     32
#define MAX_PATH_LEN     256
#define MAX_LINE_LEN     512

/* 파일/폴더 경로 상수 (init_data_dir() 이후 ~/.diary/ 기준 상대 경로) */
#define DATA_DIR    "data"
#define DIARIES_DIR "data/diaries"
#define USERS_FILE  "data/users.txt"

/* 일기 공개 설정 */
#define VISIBILITY_PUBLIC  "PUBLIC"
#define VISIBILITY_PRIVATE "PRIVATE"

/* User 구조체 - 사용자 1명의 정보 */
typedef struct {
    int  id;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char nickname[MAX_NICKNAME_LEN];
    char createdAt[MAX_DATE_LEN];
} User;

/* Diary 구조체 - 일기 1편의 정보 */
typedef struct {
    char id[MAX_ID_LEN];
    char filename[MAX_PATH_LEN];
    char author[MAX_USERNAME_LEN];
    char title[MAX_TITLE_LEN];
    char visibility[16];
    char createdAt[MAX_DATE_LEN];
    char updatedAt[MAX_DATE_LEN];
    char content[MAX_CONTENT_LEN];
} Diary;

/* 반환 코드 */
#define RET_OK    0
#define RET_FAIL -1

#endif /* COMMON_H */
