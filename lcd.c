#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <wiringPi.h>
#include <mcp23017.h>
#include <lcd.h>

#include "cpu.h"
#include "memory.h"
#include "wifi.h"
#include "lan.h"
#include "lib/filesize_h.h"
#include "lib/adafruit.h"

/**
 * Lcd file descriptor
 */
int lcd;

/**
 * Pipe file descriptor
 */
int fd[2];

/**
 * LCD vars
 */
int display = 0, lcd_timer = -1;

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

    char line1[AF_COLS], line2[AF_COLS], load[3], temp[3];
    snprintf(load, sizeof(load), "%.0f", usage);
    sprintf(line1,
        "%s%*s%%",
        "CPU load:",
        AF_COLS - strlen("CPU load:") - 1,
        load
    );
    snprintf(temp, sizeof(temp), "%.0f", cpu_usage->temp);
    sprintf(line2,
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
    struct mem_info *mem_usage = mem_get_usage();

    if (mem_usage == NULL) {
        lcdClear(lcd);
        lcdPuts(lcd, strerror(errno));
        return;
    }

    char line1[AF_COLS], line2[AF_COLS];
    sprintf(line1, "Used: %s", filesize_h(mem_usage->used));
    sprintf(line2, "Free: %s", filesize_h(mem_usage->free));

    lcdHome(lcd);
    lcdPrintf(lcd, "%*s", -AF_COLS, line1);
    lcdPrintf(lcd, "%*s", -AF_COLS, line2);

    free(mem_usage);
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

        char line1[AF_COLS], line2[AF_COLS];
        sprintf(line1, "%s", inet_ntoa(((struct sockaddr_in *) ifa->ifa_addr)->sin_addr));
        if (info != NULL) {
            sprintf(line2, "Signal: %ddBm", info->sig);
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
    for (int i = 0; i <= 4; i++) {
        pinMode(AF_BASE + i, INPUT);
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
    static int last_state;

    if (state == LCD_LED_TOGGLE) {
        state = !((bool) last_state);
    }

    digitalWrite(AF_LED, !state);

    if (state == true && last_state == false) {
        lcd_timer = 5;
    }

    last_state = state;
}

int key_listener()
{
    bool pressed[AF_KEYS_R];
    while (true) {
        for (int i = 0; i < sizeof(AF_KEYS) / sizeof(int); i++) {
            if (digitalRead(AF_KEYS[i]) == HIGH) {
                pressed[AF_KEYS[i]] = true;
            }
            if (pressed[AF_KEYS[i]] && digitalRead(AF_KEYS[i]) == LOW) {
                pressed[AF_KEYS[i]] = false;
                write(fd[1], &AF_KEYS[i], sizeof(int));
                kill(getppid(), SIGUSR1);
            }
        }
        delay(50);
    }

    return 0;
}

static void key_handler(int sig, siginfo_t *siginfo, void *context)
{
    int key;
    read(fd[0], &key, sizeof(int));

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

int lcd_display()
{
    int display_prev;

    // Start off with the display off
    lcd_led(LCD_LED_OFF);

    do {
        if (display_prev != display) {
            lcdClear(lcd);
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

    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);
        return key_listener();
    }

    close(fd[1]);

    struct sigaction act;

    act.sa_sigaction = &key_handler;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);

    // Register degree symbol
    lcdCharDef(lcd, AF_DEGREE, AF_DEGREE_DEF);

    return lcd_display();
}
