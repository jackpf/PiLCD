#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "lib/kbhit.h"

struct cpu_info;

struct cpu_info *cpu_get_usage();
void cpu_display(cpu_info *);