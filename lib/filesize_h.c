#include "filesize_h.h"

char *filesize_h(unsigned long sz)
{
    char *unit_map[] = {"GB", "MB", "KB", "bytes"};
    int i, j;

    for (i = 30, j = 0; i >= 0; i -= 10, j++) {
        if (sz >= 1 << i) {
            break;
        }
    }

    char *str = (char *) malloc(16 * sizeof(char));
    snprintf(str, 16, "%ld%s", sz / (1 << i), unit_map[j]);

    return str;
}
