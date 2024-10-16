#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>

int main() {
    int p[2];
    if(pipe(p) < 0)
        fprintf(stderr, "Failed to create pipe");

    pid_t id = fork();

    if(id == 0) {
        close(STDOUT_FILENO);
        dup(p[1]); 

        // Child
        char* args[] = { "echo", "Hello wllld\n", NULL };

        execvp("echo", args); 
    } else {
        // Parent
        printf("This is a parent process\n");

        char buffer[200];
        read(p[0], buffer, 18);

        printf("This is from child process: '%s'\n", buffer);
    }

    int status;
    waitpid(id, &status, 0); // Wait child process to exit

    if(!WIFEXITED(status))
        fprintf(stderr, "Child process terminated abnormally\n");

    close(p[0]);
    close(p[1]);

    return 0;
}