#include "cpu.h"

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

struct cpu_info *cpu_get_usage()
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

void cpu_display(cpu_info *cpu_usage)
{
    printf(
        "\rCPU load: %.0f%%, CPU temp: %.0f",
        cpu_usage->cpu_time / cpu_usage->sys_time * 100,
        cpu_usage->temp
    );

    fflush(stdout);
}
