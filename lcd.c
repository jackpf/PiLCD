#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <wiringPi.h>
#include <mcp23017.h>
#include <lcd.h>

#include "cpu.h"
#include "memory.h"
#include "wifi.h"
#include "lib/filesize_h.h"

// Defines for the Adafruit Pi LCD interface board
#define AF_BASE     100
#define AF_RED      (AF_BASE + 6)
#define AF_GREEN    (AF_BASE + 7)
#define AF_BLUE     (AF_BASE + 8)

#define AF_E        (AF_BASE + 13)
#define AF_RW       (AF_BASE + 14)
#define AF_RS       (AF_BASE + 15)

#define AF_DB4      (AF_BASE + 12)
#define AF_DB5      (AF_BASE + 11)
#define AF_DB6      (AF_BASE + 10)
#define AF_DB7      (AF_BASE +  9)

#define AF_SELECT   (AF_BASE +  0)
#define AF_RIGHT    (AF_BASE +  1)
#define AF_DOWN     (AF_BASE +  2)
#define AF_UP       (AF_BASE +  3)
#define AF_LEFT     (AF_BASE +  4)

#define I2C_ADDR    0x20

int lcd;

void cpu_display()
{
    struct cpu_info *cpu_usage = cpu_get_usage();

    lcdHome(lcd);
    lcdPrintf(lcd,
        "CPU load: %.0f%%\nCPU temp: %.0f",
        cpu_usage->cpu_time / cpu_usage->sys_time * 100,
        cpu_usage->temp
    );

    free(cpu_usage);
}

void mem_display()
{
    struct mem_info *mem_usage = mem_get_usage();

    lcdHome(lcd);
    lcdPrintf(lcd,
        "Memory load: %s",
        filesize_h(mem_usage->used)
    );

    free(mem_usage);
}

void wifi_display()
{
    static bool initialised = false;

    static struct ifaddrs *ifa;

    if (!initialised) {
        wifi_init();
        ifa = wifi_find_if();

        initialised = true;
    }

    if (ifa != NULL) {
        struct wifi_info *info = wifi_getinfo(ifa);

        if (info != NULL) {
            lcdHome(lcd);
            lcdPrintf(lcd,
                "%s\nSignal: %ddBm",
                //info->ifa_name,
                info->addr,
                info->sig
            );
        }

        free(info);
    } else {
        lcdHome(lcd);
        lcdPuts(lcd, "No interface found");
    }
}

// More info: https://github.com/Gadgetoid/WiringPi2-Python/blob/master/WiringPi/examples/lcd-adafruit.c
void lcdBacklight(int on)
{
    pinMode(AF_RED,   OUTPUT);
    pinMode(AF_GREEN, OUTPUT);
    pinMode(AF_BLUE,  OUTPUT);

    digitalWrite(AF_RED,   !(on & 1));
    digitalWrite(AF_GREEN, !(on & 2));
    digitalWrite(AF_BLUE,  !(on & 4));
}

int main(int argc, char **argv)
{
    wiringPiSetup();
    mcp23017Setup(AF_BASE, I2C_ADDR);
    int lcd = lcdInit(2, 16, 4, AF_RS, AF_E, AF_DB4, AF_DB5, AF_DB6, AF_DB7, 0, 0, 0, 0);

    lcdBacklight(true);

    typedef void (*display_func)(void);

    display_func displays[3] = {&cpu_display, &mem_display, &wifi_display};

    int i = 2;//0;

    do {
        displays[i]();

        if (*displays[i] != *cpu_display) { // Only sleep if not on cpu since it takes 1s to check the cpu info anyway
            sleep(1);
        }
    } while (true);

    lcdHome(lcd);
    lcdBacklight(false);

    return 0;
}