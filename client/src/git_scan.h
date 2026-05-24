#ifndef GIT_SCAN_H
#define GIT_SCAN_H

#include "common.h"

int cmd_git(int argc, char *argv[]);
int git_scan_collect(GitInfo *out);

#endif /* GIT_SCAN_H */
