#include <stdio.h>
#include <string.h>
#include "common.h"
#include "auth.h"
#include "team.h"
#include "log_write.h"
#include "log_read.h"
#include "git_scan.h"
#include "notify.h"
#include "session.h"

static void print_help(void);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "register") == 0) {
        return cmd_register();
    } else if (strcmp(cmd, "login") == 0) {
        return cmd_login();
    } else if (strcmp(cmd, "logout") == 0) {
        return cmd_logout();
    } else if (strcmp(cmd, "whoami") == 0) {
        return cmd_whoami();
    } else if (strcmp(cmd, "post") == 0) {
        return cmd_post();
    } else if (strcmp(cmd, "show") == 0) {
        return cmd_show(argc, argv);
    } else if (strcmp(cmd, "team") == 0) {
        return cmd_team(argc, argv);
    } else if (strcmp(cmd, "git") == 0) {
        return cmd_git(argc, argv);
    } else if (strcmp(cmd, "daemon") == 0) {
        return cmd_daemon(argc, argv);
    } else if (strcmp(cmd, "notify") == 0) {
        return cmd_notify(argc, argv);
    } else if (strcmp(cmd, "help") == 0) {
        print_help();
        return 0;
    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        print_help();
        return 1;
    }
}

static void print_help(void) {
    printf("Usage: devlog <command> [options]\n\n");
    printf("Auth:   register | login | logout | whoami\n");
    printf("Log:    post | show [--my] [--date YYYY-MM-DD]\n");
    printf("Team:   team create <name> | join <code> | info | leave\n");
    printf("Git:    git add <path> | remove <path> | list\n");
    printf("Daemon: daemon status | notify on | notify off\n");
    printf("        help\n");
}
