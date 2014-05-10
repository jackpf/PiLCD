debug = -g

main: main.c cpu.c cpu.h memory.c memory.h wifi.c wifi.h
	gcc $(debug) -lm -std=c99 cpu.c wifi.c memory.c ./lib/*.c main.c -o ./bin/main

lcd: lcd.c
	gcc $(debug) -D_POSIX_C_SOURCE=199309L -lm -lwiringPi -lwiringPiDev -std=c99 lcd.c cpu.c wifi.c lan.c memory.c ./lib/*.c -o ./bin/lcd
