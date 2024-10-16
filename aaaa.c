#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void run_command(char *cmd[], int input_fd, int output_fd) {
    if (fork() == 0) {
        // If there is an input pipe, replace stdin
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        // If there is an output pipe, replace stdout
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        // Execute the command
        execvp(cmd[0], cmd);
        perror("execvp failed");
        exit(1);
    }
}

int main() {
    int pipe1[2], pipe2[2];

    // Create the first pipe for wget -> grep
    pipe(pipe1);

    // First command: wget -q -O - https://en.wikipedia.org/wiki/Unix
    char *cmd1[] = {"wget", "-q", "-O", "-", "https://en.wikipedia.org/wiki/Unix", NULL};
    run_command(cmd1, -1, pipe1[1]);

    // Close the write end of the first pipe
    close(pipe1[1]);

    // Create the second pipe for grep -> cat
    pipe(pipe2);

    // Second command: grep -o -P '(?<=href=\")http[^\"]*'
    char *cmd2[] = {"grep", "-o", "-P", "(?<=href=\")http[^\"]*", NULL};
    run_command(cmd2, pipe1[0], pipe2[1]);

    // Close the read end of the first pipe and write end of the second pipe
    close(pipe1[0]);
    close(pipe2[1]);

    // Third command: cat -n
    char *cmd3[] = {"cat", "-n", NULL};
    run_command(cmd3, pipe2[0], -1);

    // Close the read end of the second pipe
    close(pipe2[0]);

    // Wait for all children to finish
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    return 0;
}