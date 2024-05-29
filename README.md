# Execute
```bash
./server_test.c     # press enter to switch between states
```

```bash
./client.c
```

---

```c
// in 'server_test.c'
// send command by function 'send_command'


struct Command {
    enum CommandType type;
    int msg1;
    int msg2;
    int winlose;
};

void send_command(struct Command cmd) {
    send(client_fd, &cmd, sizeof(cmd), 0);
    printf("Sent state change: %d\n", cmd.type);
}

int main(){
    struct Command command;
    send_command(command);
}

```