#include "git_scan.h"
#include <stdio.h>
#include <string.h>

/* Subcommands: add <path> | remove <path> | list */
int cmd_git(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog git <add|remove|list> [path]\n");
        return 1;
    }
    return 0;
}

/* Scans registered dirs recursively for .git; picks repo with most recent commit. */
int git_scan_collect(GitInfo *out) {
    return 0;
}
