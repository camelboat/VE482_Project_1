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

#include "function.h"

void mumsh_loop();
char* mumsh_read_line();
char** mumsh_parse_args(char* line_content);
//int* mumsh_redirection(char** line_args);
int mumsh_execute(char** line_args);
int mumsh_start_process(char **line_args);



void mumsh_loop(){
    char *line_content;
    char **line_args;
    int active = 1;

    while( active )
    {
        printf("mumsh $ ");
        line_content = mumsh_read_line();
        line_args = mumsh_parse_args(line_content);
        printf("%s %s %s %s", line_args[0], line_args[1], line_args[2], line_args[3]);
        if (!strcmp(line_content, "exit"))
        {
            active = 0;
        }
        else
        {
//            int* redirection = mumsh_redirection(line_args);
//            printf("%d", redirection[0]);
//            printf("%d", redirection[1]);
//            if (redirection[0] != -1)
//            {
//                if (redirection[0])
//                {
//                    close(1);
//                    dup2(redirection[1], 1);
//                }
//                free(redirection);
                active = mumsh_execute(line_args);
//            }
        }
    }
    free(line_content);
    free(line_args);

}

char *mumsh_read_line()
{
    size_t line_size = 1024;
    char *tmp_line = (char*)malloc(line_size);

//    getline(&tmp_line, &line_size, stdin);
//    return tmp_line;
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
        tmp_arg_all[loc] = tmp_arg;
        loc++;
    }
    tmp_arg_all[loc] = NULL;
    return tmp_arg_all;
}

//int* mumsh_redirection(char** line_args)
//{
//    int* redirection = (int*)malloc(sizeof(int) * 2);
//    redirection[0] = 0;
//    redirection[1] = 0;
////    int redirect_input = 0;
//    int loc_tmp = 0;
//    for (int i = 0; line_args[i] != NULL; ++i)
//    {
//        if (!strcmp(">", line_args[i]))
//        {
//            redirection[0]= 1;
//            loc_tmp = i;
//            break;
//        }
//    }
//    line_args[loc_tmp] = NULL;
//    if (redirection[0])
//    {
//        if (line_args[loc_tmp+1] == NULL)
//        {
//            printf("output redirection error");
//            printf("\n");
//            redirection[0] = -1;
//            return redirection;
//        }
//        close(1);
//        int fd = open(line_args[loc_tmp+1], O_WRONLY | O_CREAT, 0777);
//        line_args[loc_tmp+1] = NULL;
//        redirection[1] = dup2(1, fd);
//    }
//    return redirection;
//}

int mumsh_execute(char** line_args)
{
    if (line_args[0] == NULL)
    {
        return 1;
    }
    else
    {
        return mumsh_start_process(line_args);
    }
}

int mumsh_start_process(char** line_args)
{
    int pid, exitstatus;
    pid = fork();
    int loc_tmp = 0;
    int output_flag = 0;
    int copy_fd;
    if (pid == -1)
    {
        perror("fork failed");
        return 0;
    }
    else if (pid == 0)
    {
        for (int i = 0; line_args[i] != NULL ; ++i)
        {
            if (!strcmp(line_args[i], ">"))
            {
                loc_tmp = i;
                output_flag = 1;
                line_args[loc_tmp] = NULL;
                break;
            }
        }
        if (output_flag)
        {
            if (line_args[loc_tmp+1] == NULL)
            {
                perror("output redirection error\n");
                return 1;
            }
            close(1);
            int fd = open(line_args[loc_tmp+1], O_WRONLY | O_CREAT, 0777);
            copy_fd = dup2(1, fd);
        }
        execvp(line_args[0], line_args);
        if (output_flag)
        {
            close(1);
            dup2(copy_fd, 1);
        }
        perror("execvp failed");
        return 0;
    } else {
        while(wait(&exitstatus) != pid);
        return 1;
    }
}



#endif //PROJECT_P1_H
