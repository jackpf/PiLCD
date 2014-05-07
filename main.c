#include "cpu.h"
#include "memory.h"
#include "wifi.h"

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
        _filesize_h(mem_usage->used)
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
    do {
        cpu_display();
    } while (true);
}