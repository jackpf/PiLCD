#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

size_t get_total_memory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size / 1024;
}

size_t get_free_memory()
{
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size / 1024;
}

int main(int argc, char **argv)
{
    long double sys_time[2], idle_time[2];

    while (true) {
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

        long double sys_time_elapsed = sys_time[1] - sys_time[0];
        long double idle_time_elapsed = idle_time[1] - idle_time[0];
        long double cpu_time = sys_time_elapsed - idle_time_elapsed;

        printf("Cpu load: %.2Lf%%\tMemory used: %dKB\n", cpu_time / sys_time_elapsed * 100, get_total_memory() - get_free_memory());
    }

    return 0;
}