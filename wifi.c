#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <net/if.h>
#include <string.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>

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

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Failed to open socket\n");
    }

    /*memset(&wrq, 0, sizeof(struct iwreq));
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = 0;
    wrq.u.data.flags = 1;
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

    ioctl(skfd, SIOCGIWSTATS, &wrq);

    printf("%d\n", wrq.u.essid.length);
    printf("%.*s\n", wrq.u.essid.length, wrq.u.essid.pointer);*/

    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
        printf("Interface does not support wlan\n");
        return;
    }

    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    ioctl(skfd, SIOCGIWNWID, &wrq);
    printf("%s\n", &(wrq.u.nwid));
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

