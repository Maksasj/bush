#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t id = fork();

    if(id == 0) {
        char* args[] = { "echo", "Hello world", NULL };
        // Child
        execvp("echo", args); 
    } else {
        // Parent
    }

    int status;
    waitpid(id, &status, 0); // Wait child process to exit

    if(!WIFEXITED(status))
        fprintf(stderr, "Child process terminated abnormally\n");

    return 0;
}