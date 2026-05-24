#ifndef COMMON_H
#define COMMON_H

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
    float cpu_usage;
    float mem_usage;
} SysInfo;

typedef struct {
    char repo_path[MAX_PATH_LEN];
    char last_commit_hash[41];
    char last_commit_msg[256];
} GitInfo;

#endif /* COMMON_H */
