struct mem_info {
    long total;
    long free;
    long used;
};

struct mem_info *mem_get_usage();
void mem_display(mem_info *)