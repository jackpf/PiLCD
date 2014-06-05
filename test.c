#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>
#include <wiringPi.h>
#include <mcp23017.h>
#include <lcd.h>
#include "lib/adafruit.h"

#define LCD_BACKLIGHT_TIMER 5
#define LCD_POLL_DELAY      50
#define LCD_LED_ON          1
#define LCD_LED_OFF         0
#define LCD_LED_TOGGLE      -1
#define MUTEX_DISPLAY       0
#define MUTEX_BUS           1

/**
 * Lcd file descriptor
 */
int lcd;

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

int lcd_display()
{
    // Start off with the display off
        digitalWrite(AF_LED, 1);

    // Register degree symbol
    lcdCharDef(lcd, AF_DEGREE, AF_DEGREE_DEF);

    int i = 0;

    do {
        char line1[AF_COLS + 1], load[4];
        snprintf(load, sizeof(load), "%d", i);
        snprintf(line1, sizeof(line1),
            "%s%*s%%",
            "CPU load:",
            AF_COLS - strlen("CPU load:") - 1,
            load
        );

        lcdHome(lcd);
        lcdPrintf(lcd, "%*s", -(AF_COLS - 1), line1);

        i = (i + 1) % 101;

        sleep(1);
    } while (1);

    return 0;
}

int main(int argc, char **argv)
{
    lcd = lcd_setup();

    return lcd_display();
}
