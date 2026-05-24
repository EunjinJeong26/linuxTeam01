#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

/*
 * devlogd — background notification daemon.
 * Registered as a systemd service (see devlog.service).
 * Communicates with the CLI client via Unix Domain Socket.
 * Fires a terminal bell (\a) at the top of each hour when notify is on.
 */

static volatile int running = 1;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT,  handle_signal);

    while (running) {
        /* TODO: wait for next hour mark, then emit BEL to active TTYs */
        sleep(60);
    }

    return 0;
}
