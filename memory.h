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

struct mem_info *mem_get_usage();
char *filesize_h(long);
