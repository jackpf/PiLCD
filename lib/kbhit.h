#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <fcntl.h>

int kbhit();
int kbhit_consume();
