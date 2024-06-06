#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "Keyboard.h"

////////////////////////////////////////////////////////////////////////////////
// start code from driver_ctl.c
////////////////////////////////////////////////////////////////////////////////
// Define the device file path
#define DEVICE_FILE "/dev/gpio_matrix"

// Define the row selection patterns
const char row_patterns[2][2] = {{1, 0}, {0, 1}};

// Function to set the row pins
void set_row_pins(int fd, const char* pattern)
{
    write(fd, pattern, 2);
}

// Function to read the column pins
void read_col_pins(int fd, char* col_values)
{
    read(fd, col_values, 4);
}
////////////////////////////////////////////////////////////////////////////////
// end code from driver_ctl.c
////////////////////////////////////////////////////////////////////////////////

struct Keyboard
{
    int fd;
    int fdPipe[2];
    pthread_t thread;
};

static void *keyboardThread(void *arg)
{
    KeyboardPtr keyboard = (KeyboardPtr)arg;
    char buttonStates[2][4] = {{1, 1, 1, 1}, {1, 1, 1, 1}}; // 1 means not pressed

    char col_values[4];
    int iRow, iCol;
    char value;

    while (1)
    {
        for (iRow = 0; iRow < 2; iRow++)
        {
            // Set the row pins
            set_row_pins(keyboard->fd, row_patterns[iRow]);

            // Read the column pins
            read_col_pins(keyboard->fd, col_values);

            for (iCol = 0; iCol < 4; iCol++)
            {
                // if the button state has changed
                if (col_values[iCol] ^ buttonStates[iRow][iCol])
                {
                    buttonStates[iRow][iCol] = col_values[iCol];

                    // if the button is pressed
                    if (!buttonStates[iRow][iCol])
                    {
                        value = iRow * 4 + iCol + '1';
                        write(keyboard->fdPipe[1], &value, 1);
                    }
                }
            }

            // Sleep for a short period to debounce
            usleep(50000);
        }
    }

    return NULL;
}

static Keyboard keyboard;
static int nInstances = 0;
static void initKeyboard(KeyboardPtr keyboard)
{
    keyboard->fd = open(DEVICE_FILE, O_RDWR);
    if (keyboard->fd < 0)
    {
        perror("In initKeyboard, open");
        exit(1);
    }

    if (pipe(keyboard->fdPipe) < 0)
    {
        perror("In initKeyboard, pipe");
        exit(1);
    }

    pthread_create(&keyboard->thread, NULL, keyboardThread, keyboard);
}

static void cleanKeyboard(KeyboardPtr keyboard)
{
    pthread_cancel(keyboard->thread);
    close(keyboard->fdPipe[0]);
    close(keyboard->fdPipe[1]);
    close(keyboard->fd);
}

KeyboardPtr createKeyboard()
{
    if (nInstances == 0)
    {
        initKeyboard(&keyboard);
    }
    nInstances++;
    return &keyboard;
}

void destroyKeyboard(KeyboardPtr keyboard)
{
    nInstances--;
    if (nInstances == 0)
    {
        cleanKeyboard(keyboard);
    }
}

int keyboardGetFileDescriptor(KeyboardPtr keyboard)
{
    return keyboard->fdPipe[0];
}
