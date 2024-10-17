#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define HAUL_IMPLEMENTATION
#include "haul/haul.h"

typedef int fd_t;

typedef struct {
    fd_t read;
    fd_t write;
} pipe_t;

pid_t execute_command(char** cmd, fd_t in_fd, fd_t out_fd) {
    pid_t id = fork();

    if (id == 0) {
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

    return id;
}

void split_by_delim(vector_t* vector, char* string, char* delim) {
    char *token = strtok(string, delim);
    while (token != NULL) {
        vector_push(vector, token);

        token = strtok(NULL, delim);
    }
}

int main() {
    // Parse commands
    char line[] = "wget -q -O - https://en.wikipedia.org/wiki/Unix | grep -o -P (?<=href=\")http[^\"]* | cat -n";

    vector_t commands;
    create_vector(&commands, 10);
    split_by_delim(&commands, line, "|");

    // Create all pipes
    vector_t pipes;
    create_vector(&pipes, 10);

    for(int i = 0; i < (vector_size(&commands) - 1); ++i) {
        pipe_t* p = (pipe_t*) malloc(sizeof(pipe_t));

        if(pipe((int*) p) < 0)
            fprintf(stderr, "Failed to create pipe");
            
        vector_push(&pipes, p);
    }
    
    vector_t processes;
    create_vector(&processes, 10);

    for(int i = 0; i < vector_size(&commands); ++i) {
        char* command = (char*) vector_get(&commands, i);

        // Get arguments
        vector_t args;
        create_vector(&args, 10);
        split_by_delim(&args, command, " ");
        vector_push(&args, NULL);

        fd_t in_fd = -1;
        fd_t out_fd = -1; 

        if(i > 0) {
            pipe_t p = *((pipe_t*) vector_get(&pipes, i - 1));
            in_fd = p.read;
        }

        if(i + 1 != vector_size(&commands)) {
            pipe_t p = *((pipe_t*) vector_get(&pipes, i));
            out_fd = p.write;
        }

        pid_t* id = (pid_t*) malloc(sizeof(pid_t));
        *id = execute_command((char**) args.items, in_fd, out_fd);
        vector_push(&processes, id);

        if(in_fd != -1)
            close(in_fd);

        if(out_fd != -1)
            close(out_fd);

        // free_vector_content(&args); need to free but later
        free_vector(&args);
    }

    // Wait child process to exit
    for(int i = 0; i < vector_size(&processes); ++i) {
        pid_t* id = (pid_t*) vector_get(&processes, i);

        int status;
        waitpid(*id, &status, 0); 

        if(!WIFEXITED(status))
            fprintf(stderr, "Child processes terminated abnormally\n");
    }

    free_vector_content(&processes);
    free_vector(&processes);

    free_vector_content(&pipes);
    free_vector(&pipes);

    // free_vector_content(&commands); need to free but later
    free_vector(&commands);

    return 0;
}