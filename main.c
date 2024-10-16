#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define HAUL_IMPLEMENTATION
#include "haul/haul.h"

typedef int fd_t;
typedef int fd_pair_t[2];

void execute_command(char** cmd, fd_t in_fd, fd_t out_fd) {
    if (fork() == 0) {
        if (in_fd != -1) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        execvp(cmd[0], cmd);
        perror("execvp failed");
        exit(1);
    }
}

int main() {
    // Parse commands
    char line[] = "wget -q -O - https://en.wikipedia.org/wiki/Unix | grep -o -P (?<=href=\")http[^\"]* | cat -n";

    queue_t commands;
    create_queue(&commands, 10);

    char *token = strtok(line, "|");
    while (token != NULL) {
        queue_push(&commands, token);

        token = strtok(NULL, "|");
    }

    // Create pipes for each process
    // queue_t pipes;
    // create_queue(&pipes, 10);

    fd_pair_t p0;
    fd_pair_t p1;

    if(pipe(p0) < 0)
        fprintf(stderr, "Failed to create pipe");

    if(pipe(p1) < 0)
        fprintf(stderr, "Failed to create pipe");

    pid_t id0;
    {
        char* command = (char*) queue_get(&commands, 0);

        printf("Parsed command: %s\n", command);

        token = strtok(command, " ");

        queue_t args;
        create_queue(&args, 10);

        while (token != NULL) {
            printf("    Parsed argument: %s\n", token);
            queue_push(&args, token);
            token = strtok(NULL, " ");
        }

        queue_push(&args, NULL);

        char** argv = (char**) args.items;
        execute_command(argv, -1, p0[1]);

        close(p0[1]);
    }

    pid_t id1;
    {
        char* command = (char*) queue_get(&commands, 1);

        printf("Parsed command: %s\n", command);

        token = strtok(command, " ");

        queue_t args;
        create_queue(&args, 10);

        while (token != NULL) {
            printf("    Parsed argument: %s\n", token);
            queue_push(&args, token);
            token = strtok(NULL, " ");
        }

        queue_push(&args, NULL);

        char** argv = (char**) args.items;
        execute_command(argv, p0[0], p1[1]);

        close(p0[0]);
        close(p1[1]);
    }

    pid_t id2;
    {
    // for(int i = 1; i < (queue_size(&commands) - 1); ++i) {
        char* command = (char*) queue_get(&commands, 2);

        printf("Parsed command: %s\n", command);

        token = strtok(command, " ");

        queue_t args;
        create_queue(&args, 10);

        while (token != NULL) {
            printf("    Parsed argument: %s\n", token);
            queue_push(&args, token);
            token = strtok(NULL, " ");
        }

        queue_push(&args, NULL);

        char** argv = (char**) args.items;
        execute_command(argv, p1[0], -1);

        close(p1[0]);
    }

    printf("Marker\n");

    int status;
    waitpid(id0, &status, 0); // Wait child process to exit

    if(!WIFEXITED(status))
        fprintf(stderr, "Child processes terminated abnormally\n");

    printf("Marker 1\n");
    waitpid(id1, &status, 0); // Wait child process to exit

    if(!WIFEXITED(status))
        fprintf(stderr, "Child processes terminated abnormally\n");
            
    printf("Marker 2\n");

    waitpid(id2, &status, 0); // Wait child process to exit

    if(!WIFEXITED(status))
        fprintf(stderr, "Child processes terminated abnormally\n");

    printf("Parent exited\n");

    return 0;
}