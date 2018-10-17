//
// Created by camelboat on 18-10-2.
//

#ifndef PROJECT_READ_PARSE_H
#define PROJECT_READ_PARSE_H

#include "function.h"


char* mumsh_read_line();
int mumsh_parse_content(content_array_t *content_array, char* line_content);
int mumsh_parse_args(content_array_t *content_array, pipe_t *complete_pipes);
int mumsh_parse_quote(char* arg);
void mumsh_parse_eliminate_quote(char* arg, int loc);


void mumsh_init_array(content_array_t *content_array);
void mumsh_init_pipe(single_command_t *command);


//void mumsh_init_complete_pipe(pipe_t *complete_pipes);
//
//void mumsh_free_array(content_array_t *content_array);
//void mumsh_free_pipe(pipe_t *complete_pipes);

#endif //PROJECT_READ_PARSE_H
