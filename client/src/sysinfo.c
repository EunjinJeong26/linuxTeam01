#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sysinfo.h"

static int read_cpu_stat(long long *total, long long *idle) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        fprintf(stderr, "sysinfo: /proc/stat 열기 실패\n");
        return -1;
    }
    long long user, nice, system, idle_val, iowait, irq, softirq, steal;
    int ret = fscanf(fp, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
                     &user, &nice, &system, &idle_val, &iowait, &irq, &softirq, &steal);
    fclose(fp);
    if (ret < 8) {
        fprintf(stderr, "sysinfo: /proc/stat 파싱 실패\n");
        return -1;
    }
    *idle  = idle_val + iowait;
    *total = user + nice + system + idle_val + iowait + irq + softirq + steal;
    return 0;
}

int sysinfo_collect(SysInfo *out) {
    long long total1, idle1, total2, idle2;

    if (read_cpu_stat(&total1, &idle1) != 0) return -1;
    usleep(100000);
    if (read_cpu_stat(&total2, &idle2) != 0) return -1;

    long long delta_total = total2 - total1;
    long long delta_idle  = idle2  - idle1;
    out->cpu_usage = (delta_total <= 0) ? 0.0f
        : (float)(1.0 - (double)delta_idle / (double)delta_total) * 100.0f;

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        fprintf(stderr, "sysinfo: /proc/meminfo 열기 실패\n");
        return -1;
    }

    long long mem_total = -1, mem_available = -1;
    char key[64];
    long long value;
    while (fscanf(fp, "%63s %lld kB\n", key, &value) == 2) {
        if (mem_total < 0 && strcmp(key, "MemTotal:") == 0)
            mem_total = value;
        else if (mem_available < 0 && strcmp(key, "MemAvailable:") == 0)
            mem_available = value;
        if (mem_total >= 0 && mem_available >= 0) break;
    }
    fclose(fp);

    if (mem_total <= 0 || mem_available < 0) {
        fprintf(stderr, "sysinfo: /proc/meminfo 파싱 실패\n");
        return -1;
    }
    out->mem_usage = (float)(1.0 - (double)mem_available / (double)mem_total) * 100.0f;
    return 0;
}
