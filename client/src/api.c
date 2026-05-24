#include "api.h"

int api_post_log(const char *token, const char *json_body) {
    return 0;
}

int api_get_logs(const char *token, const char *date, int my_only, char *out_buf, int buf_size) {
    return 0;
}

int api_register(const char *username, const char *password) {
    return 0;
}

int api_login(const char *username, const char *password, char *token_out, int token_size) {
    return 0;
}

int api_team_create(const char *token, const char *name, char *invite_code_out, int buf_size) {
    return 0;
}

int api_team_join(const char *token, const char *invite_code) {
    return 0;
}

int api_team_info(const char *token, char *out_buf, int buf_size) {
    return 0;
}

int api_team_leave(const char *token) {
    return 0;
}
