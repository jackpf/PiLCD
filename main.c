#include "cpu.h"
#include "memory.h"
#include "wifi.h"

int main(int argc, char **argv)
{
    struct cpu_info *cpu_usage;

    do {
        cpu_usage = cpu_get_usage();
        cpu_display(cpu_usage);
    } while (true);

    return 0;
}
