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
#include "lib/filesize_h.h"
#include "lib/adafruit.h"

int lcd;

void cpu_display()
{
    struct cpu_info *cpu_usage = cpu_get_usage();

    float usage = cpu_usage->cpu_time / cpu_usage->sys_time * 100;

    lcdHome(lcd);
    lcdPrintf(lcd,
        "CPU load: %s%s%.0f%\nCPU temp:  %.0f",
        usage < 100 ? " " : "",
        usage < 10 ? " " : "",
        usage,
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
        "Used: %s\nFree: %s",
        filesize_h(mem_usage->used),
        filesize_h(mem_usage->free)
    );

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
    }

    if (ifa != NULL) {
        struct wifi_info *info = wifi_getinfo(ifa);

        if (info != NULL) {
            lcdHome(lcd);
            lcdPrintf(lcd,
                "%s\nSignal: %ddBm",
                info->addr,
                info->sig
            );
        }

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
    for (int i = 0 ; i <= 4 ; i++) {
        pinMode(AF_BASE + i, INPUT);
        pullUpDnControl(AF_BASE + i, PUD_UP); // Enable pull-ups, switches close to 0v
    }

    // Control signals
    pinMode(AF_RW, OUTPUT);
    digitalWrite(AF_RW, LOW); // Not used with wiringPi - always in write mode

    return lcdInit(2, 16, 4, AF_RS, AF_E, AF_DB4, AF_DB5, AF_DB6, AF_DB7, 0, 0, 0, 0);
}

void lcd_toggle_led()
{
    static int state = true;

    state = !state;

    digitalWrite(AF_LED, !state);
}

int fd[2];

int display = 0;
typedef void (*display_func)(void);
display_func displays[] = {&cpu_display, &mem_display, &wifi_display};

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
            lcd_toggle_led();
        break;
    }
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

    // Define degree symbol
    lcdCharDef(lcd, AF_DEGREE, AF_DEGREE_DEF);

    int display_prev;

    do {
        if (display_prev != display) {
            lcdClear(lcd);
        }
        display_prev = display;

        displays[display]();

        if (*displays[display] != *cpu_display) { // Only sleep if not on cpu since it takes 1s to check the cpu info anyway
            sleep(1);
        }
    } while (true);

    lcdClear(lcd);

    return 0;
}
