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

#include "function.h"

void mumsh_loop();
char* mumsh_read_line();
char** mumsh_parse_args(char* line_content);
int mumsh_execute(char** line_args);
int mumsh_start_process(char **line_args)；

char *function_list[] = {
        "ls",
        "exit"
};

int get_function_num()
{
    return sizeof(function_list) / sizeof(char*);
}





void mumsh_loop(){
    char *line_content;
    char **line_args;
    int active = 1;

    while( active )
    {
        printf("mumsh $ ");
        line_content = mumsh_read_line();
//        printf("%s", line_content);
        line_args = mumsh_parse_args(line_content);
        printf("%s", line_args[0]);
        active = mumsh_execute(line_args);
        printf("\n");
    }
    free(line_content);
    free(line_args);
}

char* mumsh_read_line()
{
    int line_size = 1024;
    char* tmp_line = (char*)malloc(line_size);
    int tmp_char;
    int loc = 0;
    while(1)
    {
        tmp_char = getchar();
        if (tmp_char == EOF || tmp_char == '\n')
        {
            tmp_line[loc] = '\0';
            return tmp_line;
        }
        else
        {
            tmp_line[loc] = tmp_char;
        }
        loc++;
    }
}

char** mumsh_parse_args(char* line_content)
{
    int arg_num = 64;
    char** tmp_arg_all = (char**)malloc(arg_num * sizeof(char*));
    char* tmp_arg;
    int loc = 0;
    const char* delim = " ";
    tmp_arg = strtok(line_content, delim);
    tmp_arg_all[loc] = tmp_arg;
    loc++;
    while (tmp_arg != NULL)
    {
        tmp_arg = strtok(NULL, delim);
        loc++;
    }
    tmp_arg_all[loc] = NULL;
    return tmp_arg_all;
}

int mumsh_execute(char** line_args)
{
    if (line_args[0] == NULL)
    {
        return 1;
    } else{
        return mumsh_start_process(line_args);
    }
}

int mumsh_start_process(char** line_args)
{
    int pid, exitstatus;
    pid = fork();
    switch(pid)
    {
        case -1:
            perror("fork failed");
            return 0;
        case 0:
            execvp(line_args[0], line_args);
            perror("execvp failed");
            return 0;
        default:
            while(wait(&exitstatus) != pid);
            printf("")

    }
}




#endif //PROJECT_P1_H
