debug = -g

main: main.c cpu.c cpu.h memory.c memory.h wifi.c wifi.h
	gcc $(debug) ./lib/kbhit.c cpu.c wifi.c memory.c main.c -o ./bin/pi-lcd -lm -std=c99
