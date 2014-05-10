#include "lan.h"

struct ifaddrs *lan_find_if()
{
    struct ifaddrs *ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, LAN_IF_NAME) == 0) {
            freeifaddrs(ifap);
            return ifa;
        }
    }

    freeifaddrs(ifap);
    return NULL;
}