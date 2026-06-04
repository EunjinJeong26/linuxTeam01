#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log_write.h"
#include "api.h"
#include "session.h"
#include "sysinfo.h"
#include "git_scan.h"
#include "common.h"

/* UTF-8 멀티바이트 문자 수 계산 (시작 바이트만 카운트) */
static int utf8_char_count(const char *s) {
    int count = 0;
    while (*s) {
        if ((*s & 0xC0) != 0x80) count++;
        s++;
    }
    return count;
}

/* 150자 초과 시 잘라내기 */
static void truncate_to_150_chars(char *s) {
    int count = 0;
    char *p = s;
    while (*p) {
        if ((*p & 0xC0) != 0x80) {
            count++;
            if (count > 150) { *p = '\0'; break; }
        }
        p++;
    }
}

/* JSON 문자열 값 이스케이프(", \\, 제어문자). dst는 충분히 커야 함. */
static void json_escape(const char *src, char *dst, int dst_size) {
    int j = 0;
    for (int i = 0; src[i] && j < dst_size - 2; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == '"' || c == '\\') {
            dst[j++] = '\\';
            dst[j++] = c;
        } else if (c == '\n') {
            dst[j++] = '\\'; dst[j++] = 'n';
        } else if (c == '\t') {
            dst[j++] = '\\'; dst[j++] = 't';
        } else if (c < 0x20) {
            /* 기타 제어문자는 생략 */
        } else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

/* /teams/me 응답에서 team_id(정수) 파싱. 실패 시 -1. */
static int parse_team_id(const char *json) {
    const char *p = strstr(json, "\"team_id\"");
    if (!p) return -1;
    p = strchr(p, ':');
    if (!p) return -1;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p < '0' || *p > '9') return -1;
    return atoi(p);
}

int cmd_post(void) {
    Session sess;
    if (session_load(&sess) != 0 || sess.token[0] == '\0') {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return -1;
    }

    /* 소속 팀 ID 확보 (서버 요청 본문에 필수) */
    char team_resp[2048];
    int rc = api_team_info(sess.token, team_resp, sizeof(team_resp));
    if (rc == -2) {
        fprintf(stderr, "팀에 소속되어 있지 않습니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "팀 정보 조회 실패\n");
        return -1;
    }
    int team_id = parse_team_id(team_resp);
    if (team_id < 0) {
        fprintf(stderr, "팀 정보 파싱 실패\n");
        return -1;
    }

    /* 시스템 정보 수집 (실패해도 계속 진행) */
    SysInfo sysinfo = { .cpu_usage = -1.0f, .mem_used_gb = -1.0f, .mem_total_gb = -1.0f };
    sysinfo_collect(&sysinfo);

    /* Git 정보 수집 (실패해도 계속 진행) */
    GitInfo gitinfo;
    memset(&gitinfo, 0, sizeof(gitinfo));
    git_scan_collect(&gitinfo);

    /* 코멘트 입력 */
    printf("오늘의 개발 로그를 입력하세요 (최대 150자, 엔터로 완료):\n");
    char comment[MAX_COMMENT_LEN + 4] = {0};
    if (fgets(comment, sizeof(comment), stdin)) {
        size_t len = strlen(comment);
        if (len > 0 && comment[len - 1] == '\n')
            comment[len - 1] = '\0';
        if (utf8_char_count(comment) > 150)
            truncate_to_150_chars(comment);
    }

    /* JSON 본문 직렬화 */
    char comment_esc[MAX_COMMENT_LEN * 2 + 4];
    json_escape(comment, comment_esc, sizeof(comment_esc));

    char branch_esc[256];
    json_escape(gitinfo.branch, branch_esc, sizeof(branch_esc));

    /* commits: 마지막 커밋 메시지 1개만 (없으면 빈 배열) */
    char commits_json[600];
    if (gitinfo.last_commit_msg[0] != '\0') {
        char msg_esc[520];
        json_escape(gitinfo.last_commit_msg, msg_esc, sizeof(msg_esc));
        snprintf(commits_json, sizeof(commits_json), "[\"%s\"]", msg_esc);
    } else {
        snprintf(commits_json, sizeof(commits_json), "[]");
    }

    char body[MAX_COMMENT_LEN * 2 + 1024];
    snprintf(body, sizeof(body),
        "{"
        "\"teamId\":%d,"
        "\"cpuUsage\":%.0f,"
        "\"memUsedGb\":%.2f,"
        "\"memTotalGb\":%.2f,"
        "\"branch\":\"%s\","
        "\"comment\":\"%s\","
        "\"commits\":%s"
        "}",
        team_id,
        sysinfo.cpu_usage < 0.0f ? 0.0f : sysinfo.cpu_usage,
        sysinfo.mem_used_gb  < 0.0f ? 0.0f : sysinfo.mem_used_gb,
        sysinfo.mem_total_gb < 0.0f ? 0.0f : sysinfo.mem_total_gb,
        branch_esc, comment_esc, commits_json);

    rc = api_post_log(sess.token, body);
    if (rc == -3) {
        fprintf(stderr, "현재 세션에 이미 게시한 로그가 있습니다.\n");
        return -1;
    }
    if (rc == -2) {
        fprintf(stderr, "팀에 소속되어 있지 않습니다.\n");
        return -1;
    }
    if (rc != 0) {
        fprintf(stderr, "로그 전송 실패\n");
        return -1;
    }

    /* 성공 출력 */
    printf("[로그 저장 완료]\n");
    if (sysinfo.cpu_usage < 0.0f)
        printf("CPU: N/A | MEM: N/A\n");
    else
        printf("CPU: %.0f%% | MEM: %.2f/%.2f GB\n",
               sysinfo.cpu_usage, sysinfo.mem_used_gb, sysinfo.mem_total_gb);

    if (gitinfo.branch[0] == '\0' && gitinfo.last_commit_msg[0] == '\0')
        printf("Git: (없음)\n");
    else
        printf("Git: [%s] %s\n", gitinfo.branch, gitinfo.last_commit_msg);

    return 0;
}
