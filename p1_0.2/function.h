//
// Created by Changxu_Luo on 18-9-19.
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
#include <errno.h>

/*
 * Define constant value
 */
#define ARG_SIZE 64
#define PIPE_SIZE 256

/*
 * Define data structures
 */
typedef struct content_array content_array_t;

struct content_array
{
    char* array[ARG_SIZE];
    int element_num;
};

typedef struct single_command single_command_t;
typedef struct pipe pipe_t;
typedef struct fork_array fork_t;
typedef struct back_job back_job_t;

// structure for the command without pipes
struct single_command
{
    int argc;
    char* argv[ARG_SIZE];
    char* f_input;
    char* f_output;
    int input; // 1: stdin 2: f_in
    int output; // 1: stdout 2: append 3: f_out
};

// structure for the command with pipes
struct pipe
{
    single_command_t commands[PIPE_SIZE];

    int pipes_num;
};

// structure for the background jobs
struct back_job
{
    single_command_t *commands[PIPE_SIZE];
};

// structure for the forked processes
struct fork_array
{
    int pid[PIPE_SIZE];
    int forks_num;
};

/*
 * main loop function for the shell
 */
void mumsh_loop(fork_t *shell_fork, int *shell_inloop);

/*
 * execute the commands in the entire line
 */
int mumsh_execute_pipes(pipe_t *complete_pipes, bool *active, fork_t *shell_fork);

/*
 * execute a single command without pipes
 */
int mumsh_execute_single_command(single_command_t *single_command,
                                int input, int output, int *pid, bool finish);

// Build-in functions
int mumsh_function_cd(single_command_t *single_command);
int mumsh_function_pwd();
int mumsh_function_jobs();

// Test functions
//void print_test(char **line_args);
//void print_test_pipes(pipe_t *complete_pipes);

#endif //PROJECT_FUNCTION_H
