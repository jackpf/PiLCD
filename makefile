debug = -g

main: main.c cpu.c cpu.h memory.c memory.h wifi.c wifi.h
	gcc $(debug) -lm -std=c99 cpu.c wifi.c memory.c ./lib/kbhit.c main.c -o ./bin/pi-lcd
