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

#define LAN_IF_NAME "eth0"

struct ifaddrs *lan_find_if();