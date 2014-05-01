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

struct ifaddrs *get_interface(char *ifname)
{
    struct ifaddrs *ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, ifname) == 0) {
            freeifaddrs(ifap);
            return ifa;
        }
    }

    freeifaddrs(ifap);
    return NULL;
}

void blah(char *ifname)
{
    int skfd;
    struct iwreq wrq;
    struct iw_statistics stats;
    struct iw_range range;
    struct iw_quality quality;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Failed to open socket: %s\n", strerror(errno));
    }

    // Check interface is wlan
    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    
    if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
        printf("Interface does not support wlan\n");
        return;
    }

    // Get signal range
    memset(&wrq, 0, sizeof(struct iwreq));
    wrq.u.data.pointer = &range;
    wrq.u.data.length = sizeof(range);
    wrq.u.data.flags = 0;
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

    if (ioctl(skfd, SIOCGIWRANGE, &wrq) < 0) {
        printf("Unable to get signal range: %s", strerror(errno));
        return;
    }

    // Get signal level
    memset(&wrq, 0, sizeof(struct iwreq));
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(stats);
    wrq.u.data.flags = 1;
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

    if (ioctl(skfd, SIOCGIWSTATS, &wrq) < 0) {
        printf("Unable to get signal level: %s\n", strerror(errno));
        return;
    }

    if ((stats.qual.updated & IW_QUAL_DBM) || (stats.qual.level > range.max_qual.level)) {
        if (!(stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
            int db_level = stats.qual.level;

            // Implement a range  of [-192; 63] dBm
            if (db_level >= 64) {
                db_level -= 0x100;
            }

            printf("level:%d dBm", db_level);
            int quality = round((db_level + 192.0) / 255.0 * 100.0);
            quality = quality < 0 ? 0 : quality > 100 ? 100 : quality;
            printf("quality: %d%%\n", quality);
        }
    }
}

int main (int argc, char **argv)
{
    struct ifaddrs *ifa = get_interface("wlan0" /*TODO: arg */);

    if (ifa != NULL) {
        struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
        char *addr = inet_ntoa(sa->sin_addr);
        printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
    } else {
        printf("No interface found\n");
    }

    blah("wlan0");

    return 0;
}

