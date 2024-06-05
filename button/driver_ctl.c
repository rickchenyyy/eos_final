#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Define the device file path
#define DEVICE_FILE "/dev/gpio_matrix"

// Define the row selection patterns
const char row_patterns[2][2] = {{1, 0}, {0, 1}};

// Function to set the row pins
void set_row_pins(int fd, const char* pattern) {
    write(fd, pattern, 2);
}

// Function to read the column pins
void read_col_pins(int fd, char* col_values) {
    read(fd, col_values, 4);
}

// Function to clear the screen and move the cursor to the top
void clear_screen() {
    printf("\033[2J\033[H");
}

int main() {
    int fd;
    char col_values[4];
    int i;

    // Open the device file
    fd = open(DEVICE_FILE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return errno;
    }

    printf("Press Ctrl+C to exit the program\n");

    while (1) {
        clear_screen();  // Clear the screen and move the cursor to the top
        for (i = 0; i < 2; i++) {
            // Set the row pins
            set_row_pins(fd, row_patterns[i]);

            // Read the column pins
            read_col_pins(fd, col_values);

            // Print the button states
            printf("Row %d: %d %d %d %d\n", i, col_values[0], col_values[1], col_values[2], col_values[3]);

            // Sleep for a short period to debounce
            usleep(50000);
        }
    }

    // Close the device file
    close(fd);
    return 0;
}
