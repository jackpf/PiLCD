#include "cpu.h"
#include "memory.h"
#include "wifi.h"
#include "lib/filesize_h.h"
#include "lib/kbhit.h"

void cpu_display()
{
    struct cpu_info *cpu_usage = cpu_get_usage();

    printf(
        "\rCPU load: %.0f%%, CPU temp: %.0f",
        cpu_usage->cpu_time / cpu_usage->sys_time * 100,
        cpu_usage->temp
    );
    fflush(stdout);

    free(cpu_usage);
}

void mem_display()
{
    struct mem_info *mem_usage = mem_get_usage();

    printf(
        "\rMemory load: %s",
        filesize_h(mem_usage->used)
    );
    fflush(stdout);

    free(mem_usage);
}

void wifi_display()
{
    static bool initialised = false;

    static struct ifaddrs *ifa;

    if (!initialised) {
        wifi_init();
        ifa = wifi_find_if();

        initialised = true;
    }

    if (ifa != NULL) {
        struct wifi_info *info = wifi_getinfo(ifa);

        printf("\rName: %s\tAddr: %s\tSignal: %ddBm", info->ifa_name, info->addr, info->sig);
        fflush(stdout);

        free(info);
    } else {
        printf("No wifi interface found\n");
    }
}

int main(int argc, char **argv)
{
    typedef void (*display_func)(void);

    display_func displays[3] = {&cpu_display, &mem_display, &wifi_display};

    int i = 0;

    do {
        displays[i]();
        sleep(1);

        if (kbhit_consume() > 0) {
            i = i < 2 ? i + 1 : 0;
        }
    } while (true);
}
