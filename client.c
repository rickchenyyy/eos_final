#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8081
#define BUF_SIZE 1024

int client_fd;
struct termios orig_termios;

void sigint_handler(int signum)
{
    close(client_fd);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);  // Restore original terminal settings
    system("xset r rate 500 30");
    printf("Client shutting down\n");
    exit(EXIT_SUCCESS);
}

enum CommandType {
    MATCHING,
    MATCHED,
    READY,
    PLAYING,
    PAUSE,
    GAMEOVER,
    EXIT
};

struct Command {
    enum CommandType type;
    int msg1;
    int msg2;
    int winlose;
};

enum CommandType current_state = MATCHING;

void handle_command(struct Command cmd) {
    current_state = cmd.type;
    switch (cmd.type) {
        case MATCHING:
            printf("State: MATCHING (Player: %d)\n", cmd.msg1);
            printf("Enter 'q' to quit\n");
            break;
        case MATCHED:
            printf("State: MATCHED (Player 1 Ready: %s, Player 2 Ready: %s)\n", cmd.msg1 ? "Yes" : "No", cmd.msg2 ? "Yes" : "No");
            printf("Enter 'r' to toggle ready/unready, 'q' to exit\n");
            break;
        case READY:
            printf("State: READY\n");
            break;
        case PLAYING:
            printf("State: PLAYING (Player 1 Score: %d, Player 2 Score: %d)\n", cmd.msg1, cmd.msg2);
            printf("Enter 'p' to pause\n");
            break;
        case PAUSE:
            printf("State: PAUSE\n");
            printf("Enter 'c' to continue, 'q' to exit\n");
            break;
        case GAMEOVER:
            printf("State: GAMEOVER (Player 1 Score: %d, Player 2 Score: %d, Winner: %s)\n", cmd.msg1, cmd.msg2, cmd.winlose == 1 ? "Player 1" : "Player 2");
            break;
        case EXIT:
            printf("State: EXIT\n");
            printf("Exiting...\n");
            sigint_handler(0);
            break;
    }
}


int initialize_client() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(SERVER_IP),
        .sin_port = htons(PORT)
    };

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    return fd;
}

void set_terminal_raw_mode() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 1;   // Minimum number of characters to read
    new_termios.c_cc[VTIME] = 0;  // Timeout for read
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    system("xset r rate 50 30");    // Set key repeat rate
}

int main()
{
    signal(SIGINT, sigint_handler);
    client_fd = initialize_client();
    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);

    struct Command cmd;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);

    set_terminal_raw_mode();

    fd_set read_fds;
    int max_fd = client_fd + 1;
    int ret;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(client_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int activity = select(max_fd, &read_fds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("select error");
        }

        if (FD_ISSET(client_fd, &read_fds)) {
            ret = recv(client_fd, &cmd, sizeof(cmd), 0);
            if(ret == 0) {
                printf("Server disconnected\n");
                sigint_handler(0);
            }
            handle_command(cmd);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input_char;
            read(STDIN_FILENO, &input_char, 1);

            int send_char = 0;
            switch (current_state) {
                case MATCHING:
                    if (input_char == 'q') send_char = 1;
                    break;
                case MATCHED:
                    if (input_char == 'r' || input_char == 'q') send_char = 1;
                    break;
                case READY:
                    // No specific chars to send
                    break;
                case PLAYING:
                    if (input_char == 'p') send_char = 1;
                    break;
                case PAUSE:
                    if (input_char == 'c' || input_char == 'q') send_char = 1;
                    break;
                case GAMEOVER:
                    // No specific chars to send
                    break;
                default:
                    break;
            }

            if (send_char) {
                send(client_fd, &input_char, 1, 0);
                printf("Sent: %c\n", input_char);
            } else {
                printf("Invalid input for current state: %c\n", input_char);
            }
        }
    }

    return 0;
}
