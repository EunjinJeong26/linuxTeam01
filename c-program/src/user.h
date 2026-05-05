#ifndef USER_H
#define USER_H

#include "common.h"

/* 회원가입 */
int  register_user(void);

/* 로그인 - 성공 시 user 구조체 채워서 반환, 실패 시 RET_FAIL */
int  login_user(User *out_user);

/* username 중복 확인 */
int  is_username_taken(const char *username);

/* users.txt에 유저 저장 */
int  save_user(const User *user);

#endif /* USER_H */
