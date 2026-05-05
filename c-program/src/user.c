<<<<<<< HEAD
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
=======
#include "user.h"

static void trim_newline(char* s) {
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

static int parse_user_line(const char* line, User* out) {
    char buf[MAX_LINE_LEN];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_newline(buf);

    int parsed = sscanf(buf,
        "%d|%31[^|]|%63[^|]|%31[^|]|%31[^\n]",
        &out->id,
        out->username,
        out->password,
        out->nickname,
        out->createdAt);

    return (parsed >= 4) ? 1 : 0;
}

static int get_next_id(void) {
    FILE* fp = fopen(USERS_FILE, "r");
    if (!fp) return 1;

    int max_id = 0;
    char line[MAX_LINE_LEN];
    User u;

    while (fgets(line, sizeof(line), fp)) {
        if (parse_user_line(line, &u) && u.id > max_id)
            max_id = u.id;
    }
    fclose(fp);
    return max_id + 1;
}

int is_username_taken(const char* username) {
    FILE* fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;

    char line[MAX_LINE_LEN];
    User u;

    while (fgets(line, sizeof(line), fp)) {
        if (parse_user_line(line, &u)) {
            if (strcmp(u.username, username) == 0) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int save_user(const User* user) {
    FILE* fp = fopen(USERS_FILE, "a");
    if (!fp) {
        perror(USERS_FILE);
        return RET_FAIL;
    }
    fprintf(fp, "%d|%s|%s|%s|%s\n",
        user->id,
        user->username,
        user->password,
        user->nickname,
        user->createdAt);
    fclose(fp);
    return RET_OK;
}

int register_user(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char nickname[MAX_NICKNAME_LEN];

    printf("\n=== 회원가입 ===\n");

    /* ── 아이디 입력 ── */
    printf("아이디: ");
    if (!fgets(username, sizeof(username), stdin)) return RET_FAIL;
    trim_newline(username);

    if (strlen(username) == 0) {
        fprintf(stderr, "[오류] 아이디를 입력해주세요.\n");
        return RET_FAIL;
    }

    /* ── 중복 확인 ── */
    if (is_username_taken(username)) {
        fprintf(stderr, "[오류] 이미 사용 중인 아이디입니다: %s\n", username);
        return RET_FAIL;
    }

    /* ── 비밀번호 입력 ── */
    printf("비밀번호: ");
    if (!fgets(password, sizeof(password), stdin)) return RET_FAIL;
    trim_newline(password);

    if (strlen(password) == 0) {
        fprintf(stderr, "[오류] 비밀번호를 입력해주세요.\n");
        return RET_FAIL;
    }

    /* ── 닉네임 입력 ── */
    printf("닉네임: ");
    if (!fgets(nickname, sizeof(nickname), stdin)) return RET_FAIL;
    trim_newline(nickname);

    if (strlen(nickname) == 0) {
        fprintf(stderr, "[오류] 닉네임을 입력해주세요.\n");
        return RET_FAIL;
    }

    /* ── User 구조체 채우기 ── */
    User user;
    memset(&user, 0, sizeof(User));

    user.id = get_next_id();
    strncpy(user.username, username, MAX_USERNAME_LEN - 1);
    strncpy(user.password, password, MAX_PASSWORD_LEN - 1);
    strncpy(user.nickname, nickname, MAX_NICKNAME_LEN - 1);

    /* 현재 시간 */
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(user.createdAt, MAX_DATE_LEN, "%Y-%m-%dT%H:%M:%S", t);

    /* ── 저장 ── */
    if (save_user(&user) == RET_FAIL) return RET_FAIL;

    printf("\n[성공] 회원가입이 완료되었습니다!\n");
    printf("  아이디 : %s\n", user.username);
    printf("  닉네임 : %s\n", user.nickname);
    printf("  가입일 : %s\n", user.createdAt);
    printf("\n이제 'diary login'으로 로그인하세요.\n\n");

    return RET_OK;
}

int login_user(User* out_user) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    printf("\n=== 로그인 ===\n");

    printf("아이디: ");
    if (!fgets(username, sizeof(username), stdin)) return RET_FAIL;
    trim_newline(username);

    printf("비밀번호: ");
    if (!fgets(password, sizeof(password), stdin)) return RET_FAIL;
    trim_newline(password);

    /* users.txt에서 일치하는 사용자 찾기 */
    FILE* fp = fopen(USERS_FILE, "r");
    if (!fp) {
        perror(USERS_FILE);
        return RET_FAIL;
    }

    char line[MAX_LINE_LEN];
    User u;

    while (fgets(line, sizeof(line), fp)) {
        if (parse_user_line(line, &u)) {
            if (strcmp(u.username, username) == 0 &&
                strcmp(u.password, password) == 0) {
                /* 비밀번호는 세션에 저장하지 않음 */
                out_user->id = u.id;
                strncpy(out_user->username, u.username, MAX_USERNAME_LEN - 1);
                strncpy(out_user->nickname, u.nickname, MAX_NICKNAME_LEN - 1);
                strncpy(out_user->createdAt, u.createdAt, MAX_DATE_LEN - 1);
                fclose(fp);
                return RET_OK;
            }
        }
    }

    fclose(fp);
    return RET_FAIL;
}
>>>>>>> 0a7b7092eeb07a062023baebf21a006ff96b68bb
