#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define HAUL_IMPLEMENTATION
#include "haul/haul.h"

// Utils
void split_by_delim(vector_t* vector, char* string, char* delim);

typedef int fd_t;

typedef struct {
    fd_t read;
    fd_t write;
} pipe_t;

typedef void (commad_callback_t)(char**, fd_t, fd_t);

typedef struct {
    char* keyword;
    commad_callback_t* callback;
} command_t;

command_t* is_build_in_command(char* cmd);

void exit_command_callback(char** cmd, fd_t in_fd, fd_t out_fd);
void cd_command_callback(char** cmd, fd_t in_fd, fd_t out_fd);

static command_t build_in_commands[] = {
    (command_t) { .keyword = "exit", .callback = &exit_command_callback },
    (command_t) { .keyword = "cd", .callback = &cd_command_callback }
};

pid_t execute_command(char** cmd, fd_t in_fd, fd_t out_fd);
void execute_line(char* line);

// bush entrypoint
int main(int argc, char *argv[]) {
    FILE *stream = stdin;
    
    if(argc >= 2)
        stream = fopen(argv[1], "r");
    else {
        printf("bush shell 1.0\n");
        printf("enter your commands (exit or '^D' if you want to exit): \n");
    }

    if(stream == NULL) {
        fprintf(stderr, "ERROR: Failed to open '%s' file\n", argv[1]);
        return 1;
    }

    char *line = NULL;
    size_t size;
    ssize_t read;

    // read input line by line and process
    while (1) {
        // if we running in terminal lets have '$' sign as our keyword
        if(stream == stdin)
            printf("\e[0;35mbush$\e[0m ");
        
        // get line from stream
        if((read = getline(&line, &size, stream)) == -1) // if EOF then returned value is -1 and we break
            break;

        // process and execute line
        line[read - 1] = '\0'; // get rid of new line character
        execute_line(line);
    }
 
    // cleanup
    if(line != NULL)
        free(line);

    if(argc > 2)
        fclose(stream);

    return 0;
}

// split string by delimiter and put into a vector
void split_by_delim(vector_t* vector, char* string, char* delim) {
    char *token = strtok(string, delim);

    while (token != NULL) {
        vector_push(vector, token);

        token = strtok(NULL, delim);
    }
}

// check if command is build in
command_t* is_build_in_command(char* cmd) {
    const int length = sizeof(build_in_commands) / sizeof(command_t);

    for(int i = 0; i < length; ++i) {
        command_t* command = &build_in_commands[i];

        if(strcmp(command->keyword, cmd) == 0)
            return command;
    }

    return NULL;
}

// build in exit command callback
void exit_command_callback(char** cmd, fd_t in_fd, fd_t out_fd) {
    exit(EXIT_SUCCESS); 
}

// build in cd command callback
void cd_command_callback(char** cmd, fd_t in_fd, fd_t out_fd) {
    if(chdir(cmd[1]) != 0)
        fprintf(stderr, "ERROR: No such file or directory");
}

// execute command
pid_t execute_command(char** cmd, fd_t in_fd, fd_t out_fd) {
    command_t* command = NULL;

    // check if this is a build in command, and if yes execute callback
    if((command = is_build_in_command(cmd[0])) != NULL) {
        (*command->callback)(cmd, in_fd, out_fd);
        return 0;
    }

    // craete child process
    pid_t id = fork();

    if (id == 0) {
        // setup child process stdin
        if (in_fd != -1) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        // setup child process stdout
        if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        // run command
        execvp(cmd[0], cmd);
        fprintf(stderr, "Command '%s' not found (execvp failed)\n", cmd[0]);
        exit(1);
    }

    return id;
}

void execute_line(char* line) {
    // extract commands
    vector_t commands;
    create_vector(&commands, 10);
    split_by_delim(&commands, line, "|");

    // create all pipes
    vector_t pipes;
    create_vector(&pipes, 10);

    for(int i = 0; i < (vector_size(&commands) - 1); ++i) {
        pipe_t* p = (pipe_t*) malloc(sizeof(pipe_t));

        if(pipe((int*) p) < 0)
            fprintf(stderr, "Failed to create pipe");

        vector_push(&pipes, p);
    }
    
    // run all processes and store them into a vector
    vector_t processes;
    create_vector(&processes, 10);

    for(int i = 0; i < vector_size(&commands); ++i) {
        char* command = (char*) vector_get(&commands, i);

        // get arguments into a vector
        vector_t args;
        create_vector(&args, 10);
        split_by_delim(&args, command, " ");
        vector_push(&args, NULL);

        fd_t in_fd = -1;
        fd_t out_fd = -1; 

        // check for pipes
        if(i > 0) {
            pipe_t p = *((pipe_t*) vector_get(&pipes, i - 1));
            in_fd = p.read;
        }

        if(i + 1 != vector_size(&commands)) {
            pipe_t p = *((pipe_t*) vector_get(&pipes, i));
            out_fd = p.write;
        }

        // execute command and save child process id
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

    // wait child process to exit
    for(int i = 0; i < vector_size(&processes); ++i) {
        pid_t* id = (pid_t*) vector_get(&processes, i);

        if(*id == 0)
            continue;

        int status; 
        waitpid(*id, &status, 0); 

        if(!WIFEXITED(status))
            fprintf(stderr, "Child processes terminated abnormally\n");
    }

    // cleanup
    free_vector_content(&processes);
    free_vector(&processes);

    free_vector_content(&pipes);
    free_vector(&pipes);

    // free_vector_content(&commands); need to free but later
    free_vector(&commands);
}
