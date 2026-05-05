#ifndef DIARY_WRITE_H
#define DIARY_WRITE_H

#include "common.h"

/* 일기 작성 */
int write_diary(const User *author);

/* 일기 파일명 생성: YYYY-MM-DD_NNN.txt */
void generate_diary_filename(const char *username, char *out, int out_size);

/* 일기 파일 저장 */
int  save_diary(const Diary *diary, const char *username);

#endif /* DIARY_WRITE_H */
