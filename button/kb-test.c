#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Keyboard.h"

int main(int argc, char const *argv[])
{
    KeyboardPtr keyboard = createKeyboard();

    int fd = keyboardGetFileDescriptor(keyboard);
    char c;

    dup2(fd, STDIN_FILENO);

    while ((c = getchar()) != EOF)
    {
        printf("Read: %c\n", c);
    }

    destroyKeyboard(keyboard);
    return 0;
}
