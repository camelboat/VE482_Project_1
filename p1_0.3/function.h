//
// Created by camelboat on 18-9-19.
//

#ifndef PROJECT_FUNCTION_H
#define PROJECT_FUNCTION_H

#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>

#define ARG_SIZE 64
#define PIPE_SIZE 256

typedef struct content_array content_array_t;

struct content_array
{
    char* array[ARG_SIZE];
    int element_num;
};

typedef struct single_command single_command_t;
typedef struct pipe pipe_t;
typedef struct fork_array fork_t;

struct single_command
{
    int argc;
    char* argv[ARG_SIZE];
    char* f_input;
    char* f_output;
    int input; // 1: stdin 2: f_in
    int output; // 1: stdout 2: append 3: f_out
};

struct pipe
{
    single_command_t commands[PIPE_SIZE];
    int pipes_num;
};

struct fork_array
{
    int pid[PIPE_SIZE];
    int forks_num;
};

void mumsh_loop(fork_t *shell_fork, int *shell_inloop);
int mumsh_execute(char** line_args);
int mumsh_start_process(char **line_args);

int mumsh_check_command(pipe_t *complete_pipes);

int mumsh_execute_pipes(pipe_t *complete_pipes, bool *active, fork_t *shell_fork);
int mumsh_execute_single_command(single_command_t *single_command,
                                int input, int output, int *pid, bool finish);

// Build-in functions
int mumsh_funcion_cd(single_command_t *single_command);
int mumsh_function_pwd();

// Interrupt handler
//sighandler_t mumsh_signal(int sig_num, void handler);


void print_test(char **line_args);
void print_test_pipes(pipe_t *complete_pipes);

#endif //PROJECT_FUNCTION_H
