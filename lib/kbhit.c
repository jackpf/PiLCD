#include "kbhit.h"

int kbhit()
{
    static const int STDIN = 0;
    static bool initialized = false;
    struct termios term;

    if (!initialized) {
        // Use termios to turn off line buffering
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);

    return bytesWaiting;
}

int kbhit_consume()
{
    int b = kbhit();

    for (int i = 0; i < b; i++) {
        getchar();
    }

    return b;
}
