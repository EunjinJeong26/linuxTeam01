#ifndef DIARY_READ_H
#define DIARY_READ_H

#include "common.h"

/* 내 일기 목록 출력 */
int  list_my_diaries(const User *user);

/* 공개 일기 목록 출력 */
int  list_public_diaries(void);

/* 일기 상세 조회 */
int  read_diary(const char *username, const char *filename, const User *viewer);

#endif /* DIARY_READ_H */
