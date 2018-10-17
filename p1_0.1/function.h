//
// Created by camelboat on 18-9-19.
//

#ifndef PROJECT_P1_H
#define PROJECT_P1_H

#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void mumsh_loop();
char* mumsh_read_line();
char*** mumsh_parse_pipe(char* line_content);
char** mumsh_parse_args(char* line_content);
int mumsh_execute(char** line_args);
int mumsh_start_process(char **line_args);

int mumsh_count_pipes(char*** line_pipes_args);

int mumsh_execute_pipes(char*** line_pipes_args);
int* mumsh_start_single_pipe(char** line_args, int *fd_last, int *fd_next);

void print_test(char **line_args);

void print_test_pipe(char ***line_pipes_args);

#endif //PROJECT_P1_H
