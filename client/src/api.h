#ifndef API_H
#define API_H

/* All server communication via libcurl goes through this module. */

int api_post_log(const char *token, const char *json_body);
int api_get_logs(const char *token, const char *date, int my_only, char *out_buf, int buf_size);
int api_register(const char *username, const char *password);
int api_login(const char *username, const char *password, char *token_out, int token_size);
int api_team_create(const char *token, const char *name, char *invite_code_out, int buf_size);
int api_team_join(const char *token, const char *invite_code);
int api_team_info(const char *token, char *out_buf, int buf_size);
int api_team_leave(const char *token, char *out_buf, int buf_size);

#endif /* API_H */
