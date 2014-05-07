#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

struct cpu_info {
    double sys_time;
    double idle_time;
    double cpu_time;
    double temp;
};

struct cpu_info *cpu_get_usage();