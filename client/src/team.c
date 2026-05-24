#include "team.h"
#include <stdio.h>
#include <string.h>

/* Subcommands: create <name> | join <code> | info | leave */
int cmd_team(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog team <create|join|info|leave> ...\n");
        return 1;
    }
    return 0;
}
