#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include<pthread.h>

#include <wiringPi.h>
#include <mcp23017.h>
#include <lcd.h>

#include "cpu.h"
#include "memory.h"
#include "wifi.h"
#include "lan.h"
#include "lib/filesize_h.h"
#include "lib/adafruit.h"

#define LCD_BACKLIGHT_TIMER 5

/**
 * Lcd file descriptor
 */
int lcd;

/**
 * Thread ids
 */
 pthread_t tid[1];

/**
 * LCD vars
 */
int display = 0, lcd_timer = -1, lcd_state;

/**
 * Display registers
 */
typedef void (*display_func)(void);
void cpu_display(void); void mem_display(void); void wifi_display(void);
display_func displays[] = {&cpu_display, &mem_display, &wifi_display};

void cpu_display()
{
    struct cpu_info *cpu_usage = cpu_get_usage();

    if (cpu_usage == NULL) {
        lcdClear(lcd);
        lcdPuts(lcd, strerror(errno));
        return;
    }

    float usage = cpu_usage->cpu_time / cpu_usage->sys_time * 100;

    char line1[AF_COLS + 1], line2[AF_COLS + 0 /* leave 1 char for degree symbol */], load[4], temp[4];
    snprintf(load, sizeof(load), "%.0f", usage);
    snprintf(line1, sizeof(line1),
        "%s%*s%%",
        "CPU load:",
        AF_COLS - strlen("CPU load:") - 1,
        load
    );
    snprintf(temp, sizeof(temp), "%.0f", cpu_usage->temp);
    snprintf(line2, sizeof(line2),
        "%s%*s",
        "CPU temp:",
        AF_COLS - strlen("CPU temp:") - 1,
        temp
    );

    lcdHome(lcd);
    lcdPrintf(lcd, "%*s", -(AF_COLS - 1), line1);
    lcdPrintf(lcd, "%*s", -(AF_COLS - 1), line2);
    lcdPutchar(lcd, AF_DEGREE);

    free(cpu_usage);
}

void mem_display()
{
    struct mem_info *mem_usage = mem_get_usage();//, *disk_usage = mem_get_disk_usage();

    if (mem_usage == NULL) {// || disk_usage == NULL) {
        lcdClear(lcd);
        lcdPuts(lcd, strerror(errno));
        return;
    }

    char line1[AF_COLS + 1], line2[AF_COLS + 1];
    snprintf(line1, sizeof(line1), "Mem: %s/%s", filesize_h(mem_usage->used), filesize_h(mem_usage->total));
    snprintf(line2, sizeof(line2), "Dsk: ...");//%s / %s", filesize_h(disk_usage->used), filesize_h(disk_usage->total));

    lcdHome(lcd);
    lcdPrintf(lcd, "%*s", -AF_COLS, line1);
    lcdPrintf(lcd, "%*s", -AF_COLS, line2);

    free(mem_usage);
    //free(disk_usage);
}

void wifi_display()
{
    static bool initialised = false;
    static struct ifaddrs *ifa;

    if (!initialised) {
        wifi_init();
        initialised = true;
    }

    if (ifa == NULL) {
        ifa = wifi_find_if();

        if (ifa == NULL) { // Fallback on eth0
            ifa = lan_find_if();
        }
    }

    if (ifa != NULL) {
        struct wifi_info *info = wifi_getinfo(ifa);

        char line1[AF_COLS + 1], line2[AF_COLS + 1];
        snprintf(line1, sizeof(line1), "%s", inet_ntoa(((struct sockaddr_in *) ifa->ifa_addr)->sin_addr));
        if (info != NULL) {
            snprintf(line2, sizeof(line2), "Signal: %ddBm", info->sig);
        }

        lcdHome(lcd);
        lcdPrintf(lcd, "%*s", -AF_COLS, line1);
        lcdPrintf(lcd, "%*s", -AF_COLS, line2);

        free(info);
    } else {
        lcdClear(lcd);
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
    for (int i = 0; i < sizeof(AF_KEYS) / sizeof(int); i++) {
        pinMode(AF_KEYS[i], INPUT);
        pullUpDnControl(AF_BASE + i, PUD_UP); // Enable pull-ups, switches close to 0v
    }

    // Control signals
    pinMode(AF_RW, OUTPUT);
    digitalWrite(AF_RW, LOW); // Not used with wiringPi - always in write mode

    return lcdInit(AF_ROWS, AF_COLS, AF_BITMODE, AF_RS, AF_E, AF_DB4, AF_DB5, AF_DB6, AF_DB7, 0, 0, 0, 0);
}

#define LCD_LED_ON      1
#define LCD_LED_OFF     0
#define LCD_LED_TOGGLE  -1

void lcd_led(int state)
{
    if (state == LCD_LED_TOGGLE) {
        state = !((bool) lcd_state);
    }

    digitalWrite(AF_LED, !state);

    if (state == true && lcd_state == false) {
        lcd_timer = LCD_BACKLIGHT_TIMER;
    }

    lcd_state = state;
}

void key_handler(int key)
{
    size_t displays_sz = sizeof(displays) / sizeof(display_func);

    switch (key) {
        case AF_LEFT:
            display = display - 1 >= 0 ? display - 1 : displays_sz - 1;
        break;
        case AF_RIGHT:
            display = display + 1 < displays_sz ? display + 1 : 0;
        break;
        case AF_SELECT:
            lcd_led(LCD_LED_TOGGLE);
        break;
    }
}

void *key_listener(void *arg)
{
    bool pressed[AF_KEYS_R] = {false};

    while (true) {
        for (int i = 0; i < sizeof(AF_KEYS) / sizeof(int); i++) {
            if (digitalRead(AF_KEYS[i]) == HIGH) {
                pressed[AF_KEYS[i]] = true;
            } else if (pressed[AF_KEYS[i]] && digitalRead(AF_KEYS[i]) == LOW) {
                pressed[AF_KEYS[i]] = false;
                key_handler(AF_KEYS[i]);
            }
        }
        delay(50);
    }
}

int lcd_display()
{
    int display_prev = -1;

    // Start off with the display off
    lcd_led(LCD_LED_OFF);

    do {
        if (display_prev != display) {
            lcdClear(lcd);
            // Keep backlight timer on for another 5 secs if it's already on
            if (lcd_state) {
                lcd_timer = LCD_BACKLIGHT_TIMER;
            }
        }
        display_prev = display;

        if (lcd_timer >= 0) {
            lcd_timer--;
            if (lcd_timer == 0) {
                lcd_led(LCD_LED_OFF);
            }
        }

        displays[display]();

        if (*displays[display] != *cpu_display) { // Only sleep if not on cpu since it takes 1s to check the cpu info anyway
            sleep(1);
        }
    } while (true);

    return 0;
}

int main(int argc, char **argv)
{
    lcd = lcd_setup();

    pthread_create(&(tid[0]), NULL, &key_listener, NULL);

    // Register degree symbol
    lcdCharDef(lcd, AF_DEGREE, AF_DEGREE_DEF);

    return lcd_display();
}
