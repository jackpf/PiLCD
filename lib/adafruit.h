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
#define AF_KEYS_R   (AF_LEFT + 1)
#define AF_NONE     0
const int AF_KEYS[] = {AF_SELECT, AF_RIGHT, AF_DOWN, AF_UP, AF_LEFT};

#define AF_ROWS		2
#define AF_COLS		16
#define AF_BITMODE	4

#define AF_DEGREE   0
uint8_t AF_DEGREE_DEF[8] = {140, 146, 146, 140, 128, 128, 128, 128}; // Custom degree symbol

#define I2C_ADDR    0x20