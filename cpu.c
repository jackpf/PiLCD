#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

struct mem_info {
    long total;
    long free;
    long used;
};

struct mem_info get_mem_usage()
{
    struct mem_info mem_usage;

    long total_pages = sysconf(_SC_PHYS_PAGES);
    long free_pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);

    mem_usage.total = total_pages * page_size / 1024;
    mem_usage.free = free_pages * page_size / 1024;
    mem_usage.used = mem_usage.total - mem_usage.free;

    return mem_usage;
}

struct cpu_info {
    double sys_time;
    double idle_time;
    double cpu_time;
    int temp;
};

struct cpu_info get_cpu_usage()
{
    long double sys_time[2], idle_time[2];
    struct cpu_info cpu_usage;

    for (int i = 0; i < 2; i++) {
        FILE *fh = fopen("/proc/uptime", "rd");
        
        char buffer[128];

        if (fgets(buffer, sizeof buffer, fh) == NULL) {
            printf("Unable to read cpu info\n");
            exit(-1);
        }

        char *sys_time_s = strtok(buffer, " ");
        char *idle_time_s = strtok(NULL, " ");

        if (sys_time_s == NULL || idle_time_s == NULL) {
            printf("Unexpected CPU info format\n");
            exit(-1);
        }

        sys_time[i] = strtod(sys_time_s, NULL);
        idle_time[i] = strtod(idle_time_s, NULL);

        fclose(fh);

        usleep(500000);
    }

    cpu_usage.sys_time = sys_time[1] - sys_time[0];
    cpu_usage.idle_time = idle_time[1] - idle_time[0];
    cpu_usage.cpu_time = cpu_usage.sys_time - cpu_usage.idle_time;

    return cpu_usage;
}

int main(int argc, char **argv)
{
    struct mem_info mem_usage;
    struct cpu_info cpu_usage;

    do {
        mem_usage = get_mem_usage();
        cpu_usage = get_cpu_usage();

        printf("CPU load: %.2f%%, Memory used: %ldKB\n", cpu_usage.cpu_time / cpu_usage.sys_time * 100, mem_usage.used);
    } while (true);

    return 0;
}