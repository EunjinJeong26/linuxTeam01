#include "diary_write.h"

/* TODO: 정은진 담당 구현 */

int write_diary(const User *author) {
    (void)author;
    return RET_FAIL;
}

void generate_diary_filename(const char *username, char *out, int out_size) {
    (void)username;
    snprintf(out, out_size, "diary.txt");
}

int save_diary(const Diary *diary, const char *username) {
    (void)diary;
    (void)username;
    return RET_FAIL;
}
