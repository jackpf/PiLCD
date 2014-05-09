PiLCD
==========

This is a small utility to display useful information on a raspberry pi LCD.
Current information available:
	- CPU usage and temperature
	- Memory usage, free memory
	- Network address and wifi signal strength

It is built for the (non RGB) Adafruit LCD plate with the MCP23017 expander chip.
It uses a fork of the wiringPi library.
The binary must be executed as root in order to interface with the GPIO ports.

Build instructions:

```shell
git clone https://github.com/jackpf/wiringPi
cd wiringPi
./build

git clone https://github.com/jackpf/PiLCD.git
cd PiLCD
make lcd # Or make main to try the console version
sudo ./bin/lcd
```

Once running, the select button toggles the backlight and the left and right buttons toggle through the various information displays.
