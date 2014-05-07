include "filesize_h.h"

char *filesize_h(long sz)
{
    char *unit_map[] = {"GB", "MB", "KB", "bytes"};
    int i, j;

    for (i = 30, j = 0; i >= 0; i -= 10, j++) {
        if (sz >= 1 << i) {
            break;
        }
    }

    char *str = (char *) malloc(strlen(unit_map[j]) + 1 + (int) log10(sz / (1 << i)));
    sprintf(str, "%ld%s", sz / (1 << i), unit_map[j]);

    return str;
}