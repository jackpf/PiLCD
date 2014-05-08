debug = -g

main: main.c cpu.c cpu.h memory.c memory.h wifi.c wifi.h
	gcc $(debug) -lm -std=c99 cpu.c wifi.c memory.c ./lib/*.c main.c -o ./bin/pi-lcd

lcd: lcd.c
	gcc $(debug) lcd.c -lwiringPi -lwiringPiDev -o ./bin/lcd
