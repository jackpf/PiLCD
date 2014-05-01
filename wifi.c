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

static int skfd;
static struct iwreq wrq;

bool wifi_init()
{
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Failed to open socket: %s\n", strerror(errno));
        return false;
    } else {
        return true;
    }
}

bool _supports_wlan(char *ifname)
{
    // Check interface is wlan
    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

    return ioctl(skfd, SIOCGIWNAME, &wrq) == 0;
}

struct ifaddrs *wifi_find_if()
{
    struct ifaddrs *ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET && _supports_wlan(ifa->ifa_name)) {
            freeifaddrs(ifap);
            return ifa;
        }
    }

    freeifaddrs(ifap);
    return NULL;
}

struct wifi_info {
    char *ifa_name;
    char *addr;
    int sig;
};

struct wifi_info *wifi_getinfo(struct ifaddrs *ifa)
{
    struct iw_statistics stats;
    struct iw_range range;
    struct iw_quality quality;
    struct wifi_info *info = (struct wifi_info *) malloc(sizeof(struct wifi_info));

    // Some interface data
    info->ifa_name = malloc(sizeof(ifa->ifa_name));
    strcpy(info->ifa_name, ifa->ifa_name);

    struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
    info->addr = (char *) malloc(sizeof(inet_ntoa(sa->sin_addr)));
    info->addr = inet_ntoa(sa->sin_addr);

    // Get signal range
    memset(&wrq, 0, sizeof(struct iwreq));
    wrq.u.data.pointer = &range;
    wrq.u.data.length = sizeof(range);
    wrq.u.data.flags = 0;
    strncpy(wrq.ifr_name, ifa->ifa_name, IFNAMSIZ);

    if (ioctl(skfd, SIOCGIWRANGE, &wrq) < 0) {
        printf("Unable to get signal range: %s", strerror(errno));
        return NULL;
    }

    // Get signal level
    memset(&wrq, 0, sizeof(struct iwreq));
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(stats);
    wrq.u.data.flags = 1;
    strncpy(wrq.ifr_name, ifa->ifa_name, IFNAMSIZ);

    if (ioctl(skfd, SIOCGIWSTATS, &wrq) < 0) {
        printf("Unable to get signal level: %s\n", strerror(errno));
        return NULL;
    }

    if ((stats.qual.updated & IW_QUAL_DBM) || (stats.qual.level > range.max_qual.level)) {
        if (!(stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
            int db_level = stats.qual.level;

            // Implement a range  of [-192; 63] dBm
            if (db_level >= 64) {
                db_level -= 0x100;
            }

            info->sig = db_level;
        }
    }
}

int main (int argc, char **argv)
{
    wifi_init();
    struct ifaddrs *ifa = wifi_find_if();

    if (ifa != NULL) {
        struct wifi_info *info = wifi_getinfo(ifa);
        printf("Name: %s\tAddr: %s\tSignal: %ddBm\n", info->ifa_name, info->addr, info->sig);
    } else {
        printf("No wifi interface found\n");
    }

    return 0;
}
