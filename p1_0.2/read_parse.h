//
// Created by Changxu_Luo on 18-10-2.
//

#ifndef PROJECT_READ_PARSE_H
#define PROJECT_READ_PARSE_H

#include "function.h"

/*
 * Read a single line, add space between different arguments,
 * also detect if there are any synatx error
 * syntax = true if there is no syntax error
 * syntax = false if there is syntax error
 */
char* mumsh_read_line(bool *syntax);

/*
 * Parse the content of a line string based on the space
 */
int mumsh_parse_content(content_array_t *content_array, char* line_content);

/*
 * Input the parsing result to the structured data for the pipes
 */
int mumsh_parse_args(content_array_t *content_array, pipe_t *complete_pipes);

/*
 * Initiate the line array
 */
void mumsh_init_array(content_array_t *content_array);

/*
 * Initiate the whole line command data structure
 */
void mumsh_init_pipe(single_command_t *command);


#endif //PROJECT_READ_PARSE_H
