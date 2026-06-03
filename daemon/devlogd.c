#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <utmp.h>

#define SOCK_PATH "/tmp/devlogd.sock"
#define BUF_SIZE  64

static volatile int running = 1;
static int notify_enabled = 1;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

static void ring_bell(void) {
    struct utmp *entry;
    char tty_path[64];
    int fd;

    setutent();
    while ((entry = getutent()) != NULL) {
        if (entry->ut_type != USER_PROCESS) continue;
        snprintf(tty_path, sizeof(tty_path), "/dev/%s", entry->ut_line);
        fd = open(tty_path, O_WRONLY | O_NOCTTY | O_NONBLOCK);
        if (fd >= 0) {
            write(fd, "\a", 1);
            close(fd);
        }
    }
    endutent();
}

static long seconds_until_next_hour(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    long secs = (long)(60 - t->tm_min) * 60 - t->tm_sec;
    return secs <= 0 ? 1 : secs;
}

static void handle_client(int client_fd) {
    char buf[BUF_SIZE];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = '\0';
    buf[strcspn(buf, "\n")] = '\0';

    if (strcmp(buf, "NOTIFY_ON") == 0) {
        notify_enabled = 1;
        write(client_fd, "OK\n", 3);
    } else if (strcmp(buf, "NOTIFY_OFF") == 0) {
        notify_enabled = 0;
        write(client_fd, "OK\n", 3);
    } else if (strcmp(buf, "STATUS") == 0) {
        const char *msg = notify_enabled
            ? "running notify=on\n"
            : "running notify=off\n";
        write(client_fd, msg, strlen(msg));
    } else {
        write(client_fd, "ERR\n", 4);
    }
}

int main(void) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT,  handle_signal);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    unlink(SOCK_PATH);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    chmod(SOCK_PATH, 0666);

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        unlink(SOCK_PATH);
        return 1;
    }

    while (running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(server_fd, &rfds);

        struct timeval tv;
        tv.tv_sec  = seconds_until_next_hour();
        tv.tv_usec = 0;

        int ret = select(server_fd + 1, &rfds, NULL, NULL, &tv);
        if (ret < 0) break;

        if (ret == 0) {
            if (notify_enabled) ring_bell();
        } else {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd >= 0) {
                handle_client(client_fd);
                close(client_fd);
            }
        }
    }

    close(server_fd);
    unlink(SOCK_PATH);
    return 0;
}
