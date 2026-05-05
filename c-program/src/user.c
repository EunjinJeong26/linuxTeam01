#include "user.h"

/* TODO: 유해린 담당 구현 */

int register_user(void) {
    /* username 중복 검사 후 users.txt에 저장 */
    return RET_FAIL;
}

int login_user(User *out_user) {
    /* users.txt 읽어서 username/password 비교 */
    (void)out_user;
    return RET_FAIL;
}

int is_username_taken(const char *username) {
    (void)username;
    return 0;
}

int save_user(const User *user) {
    (void)user;
    return RET_FAIL;
}
