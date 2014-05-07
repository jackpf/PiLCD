debug = -g

cpu: cpu.c
	gcc $(debug) ./lib/kbhit.c cpu.c -o ./bin/cpu -lm -std=c99
wifi: wifi.c
	gcc $(debug) wifi.c -o ./bin/wifi -lm -std=c99
