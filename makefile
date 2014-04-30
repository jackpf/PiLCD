debug = -g

cpu: cpu.c
	gcc $(debug) cpu.c -o cpu -lm -std=c99
wifi: wifi.c
	gcc $(debug) wifi.c -o wifi -lm -std=c99
