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
#define AF_LED      (AF_BASE + 6)

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

#define AF_DEGREE   0
uint8_t AF_DEGREE_DEF[8] = {140, 146, 146, 140, 128, 128, 128, 128};

#define I2C_ADDR    0x20

int lcd;

void cpu_display()
{
    struct cpu_info *cpu_usage = cpu_get_usage();

    lcdHome(lcd);
    lcdPrintf(lcd,
        "CPU load: %.0f%\nCPU temp: %.0f",
        cpu_usage->cpu_time / cpu_usage->sys_time * 100,
        cpu_usage->temp
    );
    lcdPutchar(lcd, AF_DEGREE);

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

int lcd_setup()
{
    wiringPiSetupSys();
    mcp23017Setup(AF_BASE, I2C_ADDR);

    // Backlight LED
    pinMode(AF_LED, OUTPUT);

    // Input buttons
    for (int i = 0 ; i <= 4 ; i++)
    {
        pinMode(AF_BASE + i, INPUT);
        pullUpDnControl(AF_BASE + i, PUD_UP); // Enable pull-ups, switches close to 0v
    }

    // Control signals
    pinMode(AF_RW, OUTPUT);
    digitalWrite(AF_RW, LOW); // Not used with wiringPi - always in write mode

    return lcdInit(2, 16, 4, AF_RS, AF_E, AF_DB4, AF_DB5, AF_DB6, AF_DB7, 0, 0, 0, 0);
}

int main(int argc, char **argv)
{
    lcd = lcd_setup();

    // Turn on LED
    digitalWrite(AF_LED, 0x0);

    lcdCharDef(lcd, AF_DEGREE, AF_DEGREE_DEF);

    typedef void (*display_func)(void);
    display_func displays[3] = {&cpu_display, &mem_display, &wifi_display};

    int i = 0;

    do {
        displays[i]();

        if (*displays[i] != *cpu_display) { // Only sleep if not on cpu since it takes 1s to check the cpu info anyway
            sleep(1);
        }
    } while (true);

    lcdHome(lcd);

    return 0;
}