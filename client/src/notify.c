#include "notify.h"
#include "api.h"
#include "session.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/devlogd.sock"
#define BUF_SIZE  64

static int send_to_daemon(const char *msg, char *resp, size_t resp_size) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    write(fd, msg, strlen(msg));

    ssize_t n = read(fd, resp, resp_size - 1);
    if (n > 0) resp[n] = '\0';
    else       resp[0] = '\0';

    close(fd);
    return 0;
}

int cmd_daemon(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[2], "status") != 0) {
        fprintf(stderr, "Usage: devlog daemon status\n");
        return 1;
    }

    char resp[BUF_SIZE];
    if (send_to_daemon("STATUS\n", resp, sizeof(resp)) < 0) {
        printf("devlogd: not running\n");
        return 1;
    }

    resp[strcspn(resp, "\n")] = '\0';
    printf("devlogd: %s\n", resp);
    return 0;
}

int cmd_notify(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: devlog notify <on|off>\n");
        return 1;
    }

    const char *arg = argv[2];
    const char *msg;
    int notify;

    if (strcmp(arg, "on") == 0) {
        msg = "NOTIFY_ON\n";
        notify = 1;
    } else if (strcmp(arg, "off") == 0) {
        msg = "NOTIFY_OFF\n";
        notify = 0;
    } else {
        fprintf(stderr, "Usage: devlog notify <on|off>\n");
        return 1;
    }

    /* 서버에 알림 설정 반영 (로그인 필요) */
    Session sess;
    if (session_load(&sess) != 0 || sess.token[0] == '\0') {
        fprintf(stderr, "로그인이 필요합니다.\n");
        return 1;
    }
    if (api_notify_set(sess.token, notify) != 0) {
        fprintf(stderr, "알림 설정 서버 반영에 실패했습니다.\n");
        return 1;
    }

    /* 로컬 데몬에 알림 스케줄 제어 전달 */
    char resp[BUF_SIZE];
    if (send_to_daemon(msg, resp, sizeof(resp)) < 0) {
        fprintf(stderr, "알림 설정은 저장되었으나 devlogd에 연결할 수 없습니다. 데몬이 실행 중인지 확인하세요.\n");
        return 1;
    }

    printf("알림 %s\n", notify ? "활성화됨" : "비활성화됨");
    return 0;
}
