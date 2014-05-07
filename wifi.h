#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <net/if.h>
#include <string.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>

struct wifi_info {
    char *ifa_name;
    char *addr;
    int sig;
};

bool wifi_init();
struct ifaddrs *wifi_find_if();
struct wifi_info *wifi_getinfo(struct ifaddrs *);