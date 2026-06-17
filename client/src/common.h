#ifndef COMMON_H
#define COMMON_H

#define API_BASE_URL "http://34.143.65.129:8080"

#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128
#define MAX_COMMENT_LEN  450  /* 150 Korean chars * 3 bytes (UTF-8) */
#define MAX_TEAM_NAME_LEN 64
#define MAX_INVITE_CODE_LEN 16
#define MAX_PATH_LEN 512
#define MAX_TOKEN_LEN 512

typedef struct {
    char username[MAX_USERNAME_LEN];
    char token[MAX_TOKEN_LEN];
} Session;

typedef struct {
    int  team_id;
    char team_name[MAX_TEAM_NAME_LEN];
    char invite_code[MAX_INVITE_CODE_LEN];
} Team;

typedef struct {
    float cpu_usage;      /* CPU 사용률 % */
    float mem_used_gb;    /* 사용 중 메모리 (GB) */
    float mem_total_gb;   /* 전체 메모리 (GB) */
} SysInfo;

typedef struct {
    char repo_path[MAX_PATH_LEN];
    char branch[128];          /* 현재 브랜치명 */
    char last_commit_msg[256]; /* 마지막 커밋 메시지 */
} GitInfo;

#endif /* COMMON_H */
