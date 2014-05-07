#include "memory.h"

struct mem_info *mem_get_usage()
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
