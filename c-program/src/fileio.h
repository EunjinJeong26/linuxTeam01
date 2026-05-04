/**
 * fileio.h - 파일/폴더 초기화 및 경로 유틸리티 함수 선언
 */

#ifndef FILEIO_H
#define FILEIO_H

#include "common.h"

/* 프로그램 시작 시 ~/.diary/ 생성 및 chdir, 하위 폴더/파일 초기화 */
int init_data_dir(void);

/* 사용자별 일기 폴더 경로 생성: data/diaries/{username} */
void build_diary_dir_path(const char *username, char *out, int out_size);

/* 일기 파일 전체 경로 생성: data/diaries/{username}/{filename} */
void build_diary_file_path(const char *username, const char *filename,
                            char *out, int out_size);

/* 경로가 존재하는 디렉터리인지 확인 (1=존재, 0=없음) */
int dir_exists(const char *path);

/* 디렉터리 생성 (이미 존재하면 RET_OK) */
int make_dir(const char *path);

#endif /* FILEIO_H */
