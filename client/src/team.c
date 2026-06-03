#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "team.h"
#include "api.h"
#include "session.h"
#include "common.h"

/*
 * JSON에서 "key":"value" 형태의 문자열 값을 추출 (경량 파싱).
 * 탐색 시작 위치(from)를 받아 다중 호출(members 배열 순회)에 사용한다.
 * 성공 시 value 뒤를 가리키는 포인터, 실패 시 NULL 반환.
 */
static const char *json_str_after(const char *from, const char *key,
                                  char *out, int out_size) {
    char pat[64];
    snprintf(pat, sizeof(pat), "\"%s\"", key);

    const char *p = strstr(from, pat);
    if (!p) return NULL;
    p += strlen(pat);

    p = strchr(p, ':');
    if (!p) return NULL;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') return NULL;
    p++;

    const char *end = strchr(p, '"');
    if (!end) return NULL;

    int len = (int)(end - p);
    if (len >= out_size) len = out_size - 1;
    memcpy(out, p, len);
    out[len] = '\0';
    return end + 1;
}

/* 로그인 세션에서 token 확보. 실패 시 -1. */
static int load_token(Session *sess) {
    if (session_load(sess) != 0 || sess->token[0] == '\0') {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }
    return 0;
}

static int team_create(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: devlog team create <name>\n");
        return -1;
    }
    const char *team_name = argv[3];

    Session sess;
    if (load_token(&sess) != 0) return -1;

    char invite_code[MAX_INVITE_CODE_LEN];
    int rc = api_team_create(sess.token, team_name,
                             invite_code, sizeof(invite_code));
    if (rc == -2) {
        fprintf(stderr, "이미 팀에 소속되어 있습니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "팀 생성 요청 실패\n");
        return -1;
    }

    printf("팀 생성 완료: %s (초대 코드: %s)\n", team_name, invite_code);
    return 0;
}

static int team_join(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: devlog team join <code>\n");
        return -1;
    }
    const char *code = argv[3];

    Session sess;
    if (load_token(&sess) != 0) return -1;

    int rc = api_team_join(sess.token, code);
    if (rc == -2) {
        fprintf(stderr, "유효하지 않은 초대 코드입니다.\n");
        return -1;
    }
    if (rc == -3) {
        fprintf(stderr, "이미 팀에 소속되었거나 팀 인원이 가득 찼습니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "팀 참가 요청 실패\n");
        return -1;
    }

    printf("팀 참가 완료\n");
    return 0;
}

static int team_info(void) {
    Session sess;
    if (load_token(&sess) != 0) return -1;

    char resp[4096];
    int rc = api_team_info(sess.token, resp, sizeof(resp));
    if (rc == -2) {
        fprintf(stderr, "소속된 팀이 없습니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "팀 정보 조회 실패\n");
        return -1;
    }

    char team_name[MAX_TEAM_NAME_LEN];
    char invite_code[MAX_INVITE_CODE_LEN];
    if (!json_str_after(resp, "name", team_name, sizeof(team_name)))
        team_name[0] = '\0';
    if (!json_str_after(resp, "invite_code", invite_code, sizeof(invite_code)))
        invite_code[0] = '\0';

    printf("팀 이름 : %s\n", team_name);
    printf("초대 코드: %s\n", invite_code);
    printf("멤버 목록:\n");

    /* members 배열에서 username/role 쌍을 순회 출력 */
    const char *cursor = strstr(resp, "\"members\"");
    if (cursor) {
        char uname[MAX_USERNAME_LEN];
        char role[32];
        const char *next;
        while ((next = json_str_after(cursor, "username", uname, sizeof(uname)))) {
            if (!json_str_after(next, "role", role, sizeof(role)))
                role[0] = '\0';
            printf("  - %s (%s)\n", uname, role);
            cursor = next;
        }
    }

    return 0;
}

static int team_leave(void) {
    Session sess;
    if (load_token(&sess) != 0) return -1;

    char resp[1024];
    int rc = api_team_leave(sess.token, resp, sizeof(resp));
    if (rc == 0) {
        printf("팀 탈퇴 완료\n");
        return 0;
    }

    /* 실패: 서버가 준 message를 그대로 출력 */
    char msg[256];
    if (json_str_after(resp, "message", msg, sizeof(msg)))
        fprintf(stderr, "%s\n", msg);
    else
        fprintf(stderr, "팀 탈퇴 요청 실패\n");
    return -1;
}

int cmd_team(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog team <create|join|info|leave> ...\n");
        return 1;
    }
    const char *sub = argv[2];
    if (strcmp(sub, "create") == 0) return team_create(argc, argv);
    if (strcmp(sub, "join")   == 0) return team_join(argc, argv);
    if (strcmp(sub, "info")   == 0) return team_info();
    if (strcmp(sub, "leave")  == 0) return team_leave();

    fprintf(stderr, "Usage: devlog team <create|join|info|leave> ...\n");
    return 1;
}
