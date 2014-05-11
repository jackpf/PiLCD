#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <sys/statvfs.h>

struct mem_info {
    unsigned long total;
    unsigned long free;
    unsigned long used;
};

struct mem_info *mem_get_usage();
struct mem_info *mem_get_disk_usage();
