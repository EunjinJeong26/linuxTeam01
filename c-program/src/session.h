/**
 * session.h - 로그인 세션 관리 함수 선언
 *
 * 세션 파일 경로: $HOME/.diary_session
 * 세션 파일 형식:
 *   id=1
 *   username=eunji
 *   nickname=은지
 *
 * 사용 흐름:
 *   diary login  -> save_session()  -> ~/.diary_session 생성
 *   diary write  -> load_session()  -> 사용자 정보 복원
 *   diary logout -> clear_session() -> ~/.diary_session 삭제
 */

#ifndef SESSION_H
#define SESSION_H

#include "common.h"

/* 세션 파일 절대 경로를 out 버퍼에 저장 */
void get_session_path(char *out, int size);

/* 로그인 성공 후 세션 파일에 사용자 정보 저장 */
int save_session(const User *user);

/* 세션 파일을 읽어서 out_user에 사용자 정보 복원 */
int load_session(User *out_user);

/* 세션 파일 삭제 (로그아웃) */
int clear_session(void);

/* 현재 로그인 상태인지 확인 (1=로그인, 0=미로그인) */
int is_logged_in(void);

#endif /* SESSION_H */
