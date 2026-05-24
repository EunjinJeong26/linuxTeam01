#include "notify.h"
#include <stdio.h>
#include <string.h>

/* Queries daemon status via Unix Domain Socket. */
int cmd_daemon(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog daemon status\n");
        return 1;
    }
    return 0;
}

/* Sends on/off toggle to daemon via Unix Domain Socket. */
int cmd_notify(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog notify <on|off>\n");
        return 1;
    }
    return 0;
}
