#include <stdio.h>
<parameter name="content">#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "log_read.h"
#include "api.h"
#include "session.h"
#include "common.h"

#define MAX_MEMBERS 6

typedef struct {
    int  user_id;
    char username[MAX_USERNAME_LEN];
} MemberEntry;

/* JSON "key":"value" 문자열 추출. from 이후 첫 매칭. 성공 시 value 뒤 포인터, 실패 시 NULL. */
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

/* JSON "key":number 추출. 실패 시 0 반환, found에 성공 여부. */
static double json_num_after(const char *from, const char *key, int *found) {
    char pat[64];
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    if (found) *found = 0;

    const char *p = strstr(from, pat);
    if (!p) return 0.0;
    p += strlen(pat);
    p = strchr(p, ':');
    if (!p) return 0.0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if ((*p < '0' || *p > '9') && *p != '-') return 0.0;
    if (found) *found = 1;
    return atof(p);
}

/* commits 배열의 첫 메시지 추출. 없으면 빈 문자열. */
static void extract_first_commit(const char *obj, char *out, int out_size) {
    out[0] = '\0';
    const char *p = strstr(obj, "\"commits\"");
    if (!p) return;
    p = strchr(p, '[');
    if (!p) return;
    /* 배열 안 첫 따옴표 문자열 */
    const char *q = strchr(p, '"');
    if (!q) return;
    /* '[' 이후에 ']'가 먼저 오면 빈 배열 */
    const char *close = strchr(p, ']');
    if (close && close < q) return;
    q++;
    const char *end = strchr(q, '"');
    if (!end) return;
    int len = (int)(end - q);
    if (len >= out_size) len = out_size - 1;
    memcpy(out, q, len);
    out[len] = '\0';
}

/* /teams/me 응답에서 (user_id, username) 멤버 맵 구성. 멤버 수 반환. */
static int build_member_map(const char *json, MemberEntry *map, int max) {
    int count = 0;
    const char *cursor = strstr(json, "\"members\"");
    if (!cursor) return 0;

    char uname[MAX_USERNAME_LEN];
    const char *next;
    while (count < max &&
           (next = json_str_after(cursor, "username", uname, sizeof(uname)))) {
        int found = 0;
        /* user_id는 같은 객체 내 username 앞에 위치 → cursor~next 구간에서 탐색 */
        int uid = (int)json_num_after(cursor, "user_id", &found);
        strncpy(map[count].username, uname, MAX_USERNAME_LEN - 1);
        map[count].username[MAX_USERNAME_LEN - 1] = '\0';
        map[count].user_id = found ? uid : -1;
        count++;
        cursor = next;
    }
    return count;
}

static const char *lookup_username(MemberEntry *map, int n, int uid, char *fallback) {
    for (int i = 0; i < n; i++)
        if (map[i].user_id == uid) return map[i].username;
    snprintf(fallback, 32, "user%d", uid);
    return fallback;
}

/* "2026-05-30T14:00" 형식 session에서 시(hour) 추출. 실패 시 -1. */
static int parse_session_hour(const char *session) {
    const char *t = strchr(session, 'T');
    if (!t || strlen(t) < 3) return -1;
    return atoi(t + 1);
}

int cmd_show(int argc, char *argv[]) {
    Session sess;
    if (session_load(&sess) != 0 || sess.token[0] == '\0') {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    /* 플래그 파싱 */
    int flag_my = 0;
    char date_buf[11] = {0};
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--my") == 0) {
            flag_my = 1;
        } else if (strcmp(argv[i], "--date") == 0 && i + 1 < argc) {
            strncpy(date_buf, argv[i + 1], 10);
            date_buf[10] = '\0';
            i++;
        }
    }

    /* 표시용 날짜(헤더): 미지정 시 오늘 */
    char header_date[11];
    if (date_buf[0] != '\0') {
        strncpy(header_date, date_buf, sizeof(header_date) - 1);
        header_date[sizeof(header_date) - 1] = '\0';
    } else {
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        strftime(header_date, sizeof(header_date), "%Y-%m-%d", lt);
    }

    /* 로그 조회 */
    char resp[16384];
    int rc = api_get_logs(sess.token, date_buf[0] ? date_buf : NULL,
                          flag_my, resp, sizeof(resp));
    if (rc == -2) {
        fprintf(stderr, "팀에 소속되어 있지 않습니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "로그 조회 실패\n");
        return -1;
    }

    /* userId → username 매핑용 멤버 맵 구성 (실패해도 fallback 출력) */
    MemberEntry members[MAX_MEMBERS];
    int member_count = 0;
    {
        char team_resp[2048];
        if (api_team_info(sess.token, team_resp, sizeof(team_resp)) == 0)
            member_count = build_member_map(team_resp, members, MAX_MEMBERS);
    }

    printf("=== %s 팀 현황 ===\n", header_date);

    /* 응답 배열 순회: 각 로그 객체는 "logId"로 시작 */
    int found = 0;
    const char *cursor = resp;
    char fallback[32];
    while ((cursor = strstr(cursor, "\"logId\"")) != NULL) {
        if (!found) printf("\n");
        found = 1;

        char session_str[32] = {0};
        json_str_after(cursor, "session", session_str, sizeof(session_str));
        int hour = parse_session_hour(session_str);

        int uid_found = 0;
        int uid = (int)json_num_after(cursor, "userId", &uid_found);
        const char *uname = uid_found
            ? lookup_username(members, member_count, uid, fallback)
            : "?";

        int cpu_f = 0, used_f = 0, total_f = 0;
        double cpu   = json_num_after(cursor, "cpuUsage",   &cpu_f);
        double used  = json_num_after(cursor, "memUsedGb",  &used_f);
        double total = json_num_after(cursor, "memTotalGb", &total_f);

        char branch[128] = {0};
        json_str_after(cursor, "branch", branch, sizeof(branch));

        char comment[MAX_COMMENT_LEN + 4] = {0};
        json_str_after(cursor, "comment", comment, sizeof(comment));

        char commit_msg[256] = {0};
        extract_first_commit(cursor, commit_msg, sizeof(commit_msg));

        if (hour >= 0) printf("[%02d시] %s\n", hour, uname);
        else           printf("[--시] %s\n", uname);

        if (cpu_f && total_f)
            printf("  CPU: %.0f%% | MEM: %.1f/%.1f GB\n", cpu, used, total);
        else
            printf("  CPU: N/A | MEM: N/A\n");

        if (branch[0] == '\0' && commit_msg[0] == '\0')
            printf("  Git: (없음)\n");
        else
            printf("  Git: [%s] %s\n", branch, commit_msg);

        if (comment[0] == '\0')
            printf("  (코멘트 없음)\n");
        else
            printf("  %s\n", comment);

        cursor += strlen("\"logId\"");
    }

    if (!found)
        printf("해당 날짜의 로그가 없습니다.\n");

    return 0;
}
