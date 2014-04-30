#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

struct mem_info {
    long total;
    long free;
    long used;
};

struct mem_info *get_mem_usage()
{
    struct mem_info *mem_usage = (struct mem_info *) malloc(sizeof(struct mem_info));

    long total_pages = sysconf(_SC_PHYS_PAGES);
    long free_pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);

    mem_usage->total = total_pages * page_size;
    mem_usage->free = free_pages * page_size;
    mem_usage->used = mem_usage->total - mem_usage->free;

    return mem_usage;
}

struct cpu_info {
    double sys_time;
    double idle_time;
    double cpu_time;
    double temp;
};

double _get_cpu_temp()
{
    FILE *fh = fopen("/sys/class/thermal/thermal_zone0/temp", "rd");

    if (fh == NULL) {
        return 0.0;
    }

    char buffer[128];

    if (fgets(buffer, sizeof buffer, fh) == NULL) {
        return 0.0;
    }

    return strtod(buffer, NULL) / 1000.0;
}

struct cpu_info *get_cpu_usage()
{
    long double sys_time[2], idle_time[2];
    struct cpu_info *cpu_usage = (struct cpu_info *) malloc(sizeof(struct cpu_info));

    for (int i = 0; i < 2; i++) {
        FILE *fh = fopen("/proc/uptime", "rd");
        
        if (fh == NULL) {
            usleep(1000000); // Artificially sleep
            return NULL;
        }

        char buffer[128];

        if (fgets(buffer, sizeof(buffer), fh) == NULL) {
            usleep(1000000); // Artificially sleep
            return NULL;
        }

        fclose(fh);

        char *sys_time_s = strtok(buffer, " ");
        char *idle_time_s = strtok(NULL, " ");

        if (sys_time_s == NULL || idle_time_s == NULL) {
            usleep(1000000); // Artificially sleep
            return NULL;
        }

        sys_time[i] = strtod(sys_time_s, NULL);
        idle_time[i] = strtod(idle_time_s, NULL);

        if (i == 0) {
            usleep(1000000);
        }
    }

    cpu_usage->sys_time = sys_time[1] - sys_time[0];
    cpu_usage->idle_time = idle_time[1] - idle_time[0];
    cpu_usage->cpu_time = cpu_usage->sys_time - cpu_usage->idle_time;
    cpu_usage->temp = _get_cpu_temp();

    return cpu_usage;
}

char *filesize_h(long sz)
{
    char *unit_map[] = {"GB", "MB", "KB", "bytes"};
    int i, j;

    for (i = 30, j = 0; i >= 0; i -= 10, j++) {
        if (sz >= 1 << i) {
            break;
        }
    }

    char *str = (char *) malloc(strlen(unit_map[j]) + 1 + (int) log10(sz / (1 << i)));
    sprintf(str, "%ld%s", sz / (1 << i), unit_map[j]);

    return str;
}

int main(int argc, char **argv)
{
    struct mem_info *mem_usage;
    struct cpu_info *cpu_usage;

    do {
        mem_usage = get_mem_usage();
        cpu_usage = get_cpu_usage();

        if (mem_usage == NULL) {
            printf("Memory load read error\n");
        } else if (cpu_usage == NULL) {
            printf("CPU load read error\n");
        } else {
            printf(
                "CPU load: %.0f%%, CPU temp: %.0f, Memory load: %s\n",
                cpu_usage->cpu_time / cpu_usage->sys_time * 100,
                cpu_usage->temp,
                filesize_h(mem_usage->used)
            );
        }
    } while (true);

    return 0;
}
