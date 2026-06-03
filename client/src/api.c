#include <stdio.h>
<parameter name="content">#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "api.h"
#include "common.h"

/* libcurl 응답 본문을 고정 버퍼에 수집하는 콜백 */
typedef struct {
    char  *buf;
    size_t size;     /* 버퍼 용량 (NUL 포함) */
    size_t len;      /* 현재까지 쓴 길이 */
} RespBuf;

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t incoming = size * nmemb;
    RespBuf *r = (RespBuf *)userdata;

    /* 버퍼가 없으면 모두 버리되 수신은 계속 진행 */
    if (!r->buf || r->size == 0) return incoming;

    size_t space = r->size - 1 - r->len;   /* NUL 자리 제외 */
    size_t copy  = incoming < space ? incoming : space;
    if (copy > 0) {
        memcpy(r->buf + r->len, ptr, copy);
        r->len += copy;
        r->buf[r->len] = '\0';
    }
    return incoming;   /* 실제 수신량 반환 (초과분은 조용히 절단) */
}

/*
 * 공통 HTTP 요청 헬퍼.
 *   method  : "GET" | "POST" | "PATCH" | "DELETE" 등
 *   path    : "/auth/login" 같은 경로 (API_BASE_URL에 이어붙임)
 *   token   : NULL이면 Authorization 헤더 생략
 *   body    : NULL이면 본문 생략 (GET 등)
 *   out_buf : 응답 본문 수집 버퍼 (NULL이면 응답 폐기)
 *   buf_size: out_buf 용량
 * 반환: HTTP 상태 코드(200/201/4xx 등), 전송 실패 시 -1
 */
static int http_request(const char *method, const char *path,
                        const char *token, const char *body,
                        char *out_buf, int buf_size) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    char url[512];
    snprintf(url, sizeof(url), "%s%s", API_BASE_URL, path);

    RespBuf resp = { out_buf, (size_t)buf_size, 0 };
    if (out_buf && buf_size > 0) out_buf[0] = '\0';

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char auth_hdr[MAX_TOKEN_LEN + 32];
    if (token && token[0] != '\0') {
        snprintf(auth_hdr, sizeof(auth_hdr), "Authorization: Bearer %s", token);
        headers = curl_slist_append(headers, auth_hdr);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    if (body) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
    }

    CURLcode rc = curl_easy_perform(curl);
    long status = -1;
    if (rc == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (rc == CURLE_OK) ? (int)status : -1;
}

/*
 * JSON 문자열 값에서 "key":"value" 형태의 value를 추출 (경량 파싱).
 * 성공 시 0, 실패 시 -1. 이스케이프는 처리하지 않음(토큰/username엔 불필요).
 */
static int json_extract_string(const char *json, const char *key,
                               char *out, int out_size) {
    char pat[64];
    snprintf(pat, sizeof(pat), "\"%s\"", key);

    const char *p = strstr(json, pat);
    if (!p) return -1;
    p += strlen(pat);

    /* ':' 까지 이동 */
    p = strchr(p, ':');
    if (!p) return -1;
    p++;

    /* 여는 따옴표까지 공백 건너뛰기 */
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') return -1;
    p++;

    const char *end = strchr(p, '"');
    if (!end) return -1;

    int len = (int)(end - p);
    if (len >= out_size) len = out_size - 1;
    memcpy(out, p, len);
    out[len] = '\0';
    return 0;
}

/* username/password로 JSON 본문 작성 */
static void build_credentials_body(char *buf, int size,
                                   const char *username, const char *password) {
    snprintf(buf, size, "{\"username\":\"%s\",\"password\":\"%s\"}",
             username, password);
}

int api_register(const char *username, const char *password) {
    char body[MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 64];
    build_credentials_body(body, sizeof(body), username, password);

    int status = http_request("POST", "/auth/register", NULL, body, NULL, 0);
    if (status == 201 || status == 200) return 0;
    if (status == 409) return -2;
    return -1;
}

int api_login(const char *username, const char *password,
              char *token_out, int token_size) {
    char body[MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 64];
    build_credentials_body(body, sizeof(body), username, password);

    char resp[1024];
    int status = http_request("POST", "/auth/login", NULL, body,
                              resp, sizeof(resp));
    if (status != 200) return -1;

    if (json_extract_string(resp, "token", token_out, token_size) != 0)
        return -1;

    return 0;
}

int api_post_log(const char *token, const char *json_body) {
    (void)token; (void)json_body;
    return 0;
}

int api_get_logs(const char *token, const char *date, int my_only,
                 char *out_buf, int buf_size) {
    (void)token; (void)date; (void)my_only; (void)out_buf; (void)buf_size;
    return 0;
}

int api_team_create(const char *token, const char *name,
                    char *invite_code_out, int buf_size) {
    char body[MAX_TEAM_NAME_LEN + 32];
    snprintf(body, sizeof(body), "{\"name\":\"%s\"}", name);

    char resp[1024];
    int status = http_request("POST", "/teams", token, body,
                              resp, sizeof(resp));
    if (status == 201 || status == 200) {
        if (json_extract_string(resp, "invite_code", invite_code_out, buf_size) != 0)
            return -1;
        return 0;
    }
    if (status == 409) return -2;   /* 이미 팀에 소속 */
    return -1;
}

int api_team_join(const char *token, const char *invite_code) {
    char body[MAX_INVITE_CODE_LEN + 32];
    snprintf(body, sizeof(body), "{\"invite_code\":\"%s\"}", invite_code);

    int status = http_request("POST", "/teams/join", token, body, NULL, 0);
    if (status == 200 || status == 201) return 0;
    if (status == 404) return -2;   /* 존재하지 않는 초대 코드 */
    if (status == 409) return -3;   /* 이미 소속 또는 인원 초과 (통합) */
    return -1;
}

int api_team_info(const char *token, char *out_buf, int buf_size) {
    int status = http_request("GET", "/teams/me", token, NULL,
                              out_buf, buf_size);
    if (status == 200) return 0;
    if (status == 404) return -2;   /* 소속된 팀 없음 */
    return -1;
}

int api_team_leave(const char *token, char *out_buf, int buf_size) {
    int status = http_request("DELETE", "/teams/me/delete", token, NULL,
                              out_buf, buf_size);
    if (status == 200) return 0;
    return -1;   /* 실패 사유는 out_buf의 응답 본문으로 전달 */
}
