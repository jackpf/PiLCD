#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <time.h>

#define MCP 0x20                // I2C bus address of MCP23017

// Port A layout
#define BUTTONS 0b00011111      // 0..4 buttons: sel, right, down, up, left
// 6 red backlight
// 7 green backlight
// Port B layout
#define LED_BLUE 0b00000001     // 0 blue backlight
#define DATA_BITS 0b00011110    // 1..4 LCD data bits D7..D4 reversed
#define BIT_D7 0b00000010    // 4 Pin D7 of LCD
#define STROBE 0b00100000       // 5 Pin E of LCD
#define LCD_RW 0b01000000       // 6 R/W pin of LCD
#define LCD_RS 0b10000000       // 7 RS pin of LCD
#define IODIRA_DEF 0b00111111   // R+G LEDs=outputs, buttons=inputs
#define IODIRB_DEF 0         // B LED, LCD pins=outputs

#define IOCON_BANK0 0x0a        // Address of IOCON when Bank 0 active
#define IOCON_BANK1 0x15        // Address of IOCON when Bank 1 active
// Addresses when Bank 1 is active
#define GPIOA 0x09
#define IODIRA 0x00
#define GPIOB 0x19
#define IODIRB 0x10

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02

#define ENTRY_MODE_SET 0x04
  #define ENTRY_RIGHT 0x00
  #define ENTRY_LEFT 0x02
  #define ENTRY_SHIFT_INC 0x01
  #define ENTRY_SHIFT_DEC 0x00

#define DISPLAY_CTRL 0x08
  #define DISPLAY_ON 0x04
  #define CURSOR_ON 0x02
  #define BLINK_ON 0x01

#define CURSOR_SHIFT 0x10
  #define DISPLAY_MOVE 0x08
  #define CURSOR_MOVE  0x00
  #define MOVE_RIGHT 0x04
  #define MOVE_LEFT 0x00

#define FUNCTION_SET 0x20

#define SET_CGRAM_ADDR 0x40

#define SET_DDRAM_ADDR 0x80
  #define LINE1 0x00
  #define LINE2 0x40

int i2c;
int device;
int led_b;
static uint8_t xlate[] = {
     0b00000000, 0b00010000, 0b00001000, 0b00011000,
     0b00000100, 0b00010100, 0b00001100, 0b00011100,
     0b00000010, 0b00010010, 0b00001010, 0b00011010,
     0b00000110, 0b00010110, 0b00001110, 0b00011110
};

void panic(char *msg) {
     perror(msg);
     exit(1);
}

void init_i2c(void) {
     i2c = open("/dev/i2c-1", O_RDWR);
     if (i2c < 0) panic("opening i2c");
     device = -1;
}

int i2c_transfer(uint8_t rw, uint8_t cmd, int size, 
                 union i2c_smbus_data *data) {
     struct i2c_smbus_ioctl_data param;
     param.read_write = rw;
     param.command = cmd;
     param.size = size;
     param.data = data;
     return ioctl(i2c, I2C_SMBUS, &param);
}

void i2c_select(int dev) {
     if (device == dev) return;
     if (ioctl(i2c, I2C_SLAVE, dev) < 0) panic("device select");
     device = dev;
}

void i2c_write_block_data(int dev, int cmd, uint8_t *buf, int n) {
     int i;
     union i2c_smbus_data data;
     data.block[0] = n;
     for (i = 0; i < n; i++) data.block[i+1] = buf[i];
     if (i2c_transfer(I2C_SMBUS_WRITE, cmd, I2C_SMBUS_BLOCK_DATA, &data) < 0)
          panic("write block");
}

void i2c_write_byte_data(int dev, int cmd, int val) {
     union i2c_smbus_data data;
     data.byte = val;
     i2c_select(dev);
     if (i2c_transfer(I2C_SMBUS_WRITE, cmd, I2C_SMBUS_BYTE_DATA, &data) < 0) 
          panic("write byte data");
}

void init_mcp(void)
{
    // Set to Bank 0, sequential mode.  Causes a minor glitch by writing
    // to OLATB if already on Bank 0.
    i2c_write_byte_data(MCP, IOCON_BANK1, 0);
}

void i2c_write_byte(int dev, int val) {
     union i2c_smbus_data data;
     data.byte = val;
     i2c_select(dev);
     if (i2c_transfer(I2C_SMBUS_WRITE, val, I2C_SMBUS_BYTE, NULL) < 0) 
          panic("write byte");
}

uint8_t i2c_read_byte(int dev) {
     union i2c_smbus_data data;
     i2c_select(dev);
     if (i2c_transfer(I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data) < 0)
          panic("read byte");
     return data.byte & 0xff;
}

void lcd_wait(void) {
     int val = led_b | LCD_RW;
     int bits;
     // Set pin D4..7 to input
     i2c_write_byte_data(MCP, IODIRB, (IODIRB_DEF | DATA_BITS));
     // Poll the pin until it is clear
     i2c_write_byte_data(MCP, GPIOB, val);
     do {
          i2c_write_byte(MCP, val|STROBE);
          bits = i2c_read_byte(MCP);
          i2c_write_byte(MCP, val);
          i2c_write_byte(MCP, val|STROBE);
          // ignore second nybble
          i2c_write_byte(MCP, val);
     } while ((bits & BIT_D7) != 0);
     // Set pins D4..7 back to output
     i2c_write_byte_data(MCP, IODIRB, IODIRB_DEF);
}
void lcd_write(int byte) {
     uint8_t buf[4];
     lcd_expand(byte, 0, buf);
     i2c_write_block_data(MCP, GPIOB, buf, 4);
}
void lcd_expand(int byte, int extra, uint8_t *buf) {
     extra |= led_b;
     int hi = (xlate[(byte>>4)&0xf] | extra);
     int lo = (xlate[byte&0xf] | extra);
     buf[0] = (hi|STROBE); buf[1] = hi;
     buf[2] = (lo|STROBE); buf[3] = lo;
}
void lcd_home(void) {
     lcd_write(RETURN_HOME);
     lcd_wait();
}
void lcd_clear(void) {
     lcd_write(CLEAR_DISPLAY);
     lcd_wait();
}
void init_lcd(void) {
     lcd_write(0x33);           // Voodoo initalisation sequence
     lcd_write(0x32);
     lcd_write(0x28);           // 2 lines of 5x8 characters
     lcd_clear();
     lcd_write(CURSOR_SHIFT | CURSOR_MOVE | MOVE_RIGHT);
     lcd_write(ENTRY_MODE_SET | ENTRY_LEFT | ENTRY_SHIFT_DEC);
     lcd_write(DISPLAY_CTRL | DISPLAY_ON);
     lcd_home();
}
void lcd_newchar(int code, int row0, int row1, int row2, int row3,
                 int row4, int row5, int row6, int row7) {
     char buf[32];
     int extra = led_b | LCD_RS;

     lcd_write(SET_CGRAM_ADDR | ((code&7) << 3));
     lcd_expand(row0, extra, &buf[0]);
     lcd_expand(row1, extra, &buf[4]);
     lcd_expand(row2, extra, &buf[8]);
     lcd_expand(row3, extra, &buf[12]);
     lcd_expand(row4, extra, &buf[16]);
     lcd_expand(row5, extra, &buf[20]);
     lcd_expand(row6, extra, &buf[24]);
     lcd_expand(row7, extra, &buf[28]);
     i2c_write_block_data(MCP, GPIOB, buf, 32);
     lcd_write(SET_DDRAM_ADDR);
}
int main(int argc, char **argv)
{
    init_i2c();
    init_mcp();
    init_lcd();

    lcd_newchar(1,
                 0b00000,
                 0b00000,
                 0b01010,
                 0b11111,
                 0b11111,
                 0b01110,
                 0b00100,
                 0b00000);
     lcd_newchar(2,
                 0b00000,
                 0b00000,
                 0b01110,
                 0b00100,
                 0b00100,
                 0b00100,
                 0b01110,
                 0b00000);

    return 0;
}

