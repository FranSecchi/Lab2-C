#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <stdint.h>

uint8_t operand1, operand2;
char operation;
int pipe_fd[2];

void generate_random_values() {
    operand1 = (rand() % 101); // generate operand1 between 0-100
    operand2 = (rand() % 101); // generate operand2 between 0-100

    switch (rand() % 4) {// generate random operation code
        case 0:
            operation = '+';
            break;
        case 1:
            operation = '-';
            break;
        case 2:
            operation = '*';
            break;
        case 3:
            operation = '/';
            break;
        default:
            operation = '+';
            break;
    }
}

void parent_process(int iterations) {
    close(pipe_fd[0]); // close unused read end of the pipe
    printf("parent (pid = %d) begins.\n", getpid());

    for (int i = 0; i < iterations; i++) {
        generate_random_values();
        printf("parent (pid = %d): iteration %d.\n", getpid(), i);

        // write operand1, operation and operand2 to the pipe
        write(pipe_fd[1], &operand1, sizeof(operand1));
        write(pipe_fd[1], &operation, sizeof(operation));
        write(pipe_fd[1], &operand2, sizeof(operand2));

        if (operation == '+') {
            printf("parent (pid = %d): %d + %d = ?\n", getpid(), operand1, operand2);
        }
        else if (operation == '-') {
            printf("parent (pid = %d): %d - %d = ?\n", getpid(), operand1, operand2);
        }
        else if (operation == '*') {
            printf("parent (pid = %d): %d * %d = ?\n", getpid(), operand1, operand2);
        }
        else if (operation == '/') {
            printf("parent (pid = %d): %d / %d = ?\n", getpid(), operand1, operand2);
        }
        else {
            printf("parent (pid = %d): Invalid operation %c\n", getpid(), operation);
        }

        sleep(1);
    }

    close(pipe_fd[1]); // close write end of the pipe
    printf("parent (pid=%d) ends.\n", getpid());
}

void child_process(int iterations) {
    close(pipe_fd[1]); // close unused write end of the pipe
    printf("child (pid = %d) begins.\n", getpid());

    uint8_t a, b;
    char op;

    for (int i = 0; i < iterations; i++) {
        read(pipe_fd[0], &a, sizeof(a));
        read(pipe_fd[0], &op, sizeof(op));
        read(pipe_fd[0], &b, sizeof(b));

        int result;
        switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/': result = a / b; break;
        }
         printf("child (pid = %d): %d %c %d = %d\n", getpid(), a, op, b, result);
    }
    close(pipe_fd[0]); // close read end of pipe
    printf("child (pid=%d) ends.\n", getpid());
}
int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 2) {
        printf("Usage: %s <num_iterations>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_iterations = atoi(argv[1]);
    pid_t pid;

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    printf("main: created pipe.\n");

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) { // parent process
        parent_process(num_iterations);
        exit(EXIT_SUCCESS);
    }
    else { // child process
        child_process(num_iterations);
        exit(EXIT_SUCCESS);
    }
    return 0;
}
