#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <net/if.h>

struct ifaddrs *get_interface(char *name)
{
    struct ifaddrs *ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, name) == 0) {
            freeifaddrs(ifap);
            return ifa;
        }
    }

    freeifaddrs(ifap);
    return NULL;
}

int main (int argc, char **argv)
{
    struct ifaddrs *ifa = get_interface("eth0" /*TODO: arg */);

    if (ifa != NULL) {
        struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
        char *addr = inet_ntoa(sa->sin_addr);
        printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
    } else {
        printf("No interface found\n");
    }

    return 0;
}