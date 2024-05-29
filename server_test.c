#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define PORT 8081
#define BUF_SIZE 1024

int server_fd, client_fd;

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

void sigint_handler(int signum)
{
    close(client_fd);
    close(server_fd);
    printf("Server shutting down\n");
    exit(EXIT_SUCCESS);
}

int initialize_server() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };

    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    return fd;
}

void send_command(struct Command cmd) {
    send(client_fd, &cmd, sizeof(cmd), 0);
    printf("Sent state change: %d\n", cmd.type);
}

void initialize_and_accept_connection() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = initialize_server();
    printf("Server listening on port %d\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

}


int main()
{
    signal(SIGINT, sigint_handler);
    initialize_and_accept_connection();

    enum CommandType state = MATCHING;

    // Define a set of test commands
    struct Command test_commands[] = {
        {MATCHING, 0, 0, 0},
        {MATCHED, 1, 0, 0},
        {READY, 0, 0, 0},
        {PLAYING, 10, 20, 0},
        {PAUSE, 0, 0, 0},
        {GAMEOVER, 15, 30, 2},
        {EXIT, 0, 0, 0}
    };

    int num_tests = sizeof(test_commands) / sizeof(test_commands[0]);
    int test_index = 0;
    // Declare buffer for receiving data
    char buf[BUF_SIZE];

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_fd, &read_fds);
        int max_fd = client_fd + 1;

        int activity = select(max_fd, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        if (FD_ISSET(client_fd, &read_fds)) {
            int bytes_read = recv(client_fd, buf, BUF_SIZE, 0);
            if (bytes_read > 0) {
                buf[bytes_read] = '\0';
                printf("Received command: %s\n", buf);
            } else if (bytes_read == 0) {
                printf("Client disconnected\n");
                break;
            } else {
                perror("recv error");
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input_char;
            read(STDIN_FILENO, &input_char, 1);
            if (input_char == '\n') {
                // Send the next test command
                send_command(test_commands[test_index]);
                test_index = (test_index + 1) % num_tests;  // Cycle through the test commands
            }
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
