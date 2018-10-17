//
// Created by camelboat on 18-10-2.
//

#include "read_parse.h"
#include "function.h"

int reading;

char *mumsh_read_line()
{
    extern int reading;

    extern int shell_inloop;
    size_t line_size = 1024;
    char *tmp_line = (char*)malloc(line_size);
    int tmp_char;
    int loc = 0;

    int quote_double = 0;
    int quote_single = 0;

//    int pipe_in_quote = 0;

    reading = 1;
    while(1)
    {
//        if (shell_inloop == -1)
//        {
//            return tmp_line;
//        }
        tmp_char = getchar();

        if (tmp_char == '\"')
        {
            if (quote_single == 0)
            {
                if (quote_double == 0)
                {
                    quote_double = 1;
                    continue;
                }
                if (quote_double == 1)
                {
//                    pipe_in_quote = 0;
                    quote_double = 0;
                    continue;
                }
            }
//            if (quote_single == 1)
//            {
//                tmp_line[loc] = '\"';
//                loc++;
//            }
        }

        if (tmp_char == '\'')
        {
            if (quote_double == 0)
            {
                if (quote_single == 0)
                {
                    quote_single = 1;
                    continue;
                }
                if (quote_single == 1)
                {
                    quote_single = 0;
//                    pipe_in_quote = 0;
                    continue;
                }
            }
//            if (quote_double == 1)
//            {
//                tmp_line[loc] = '\'';
//                loc++;
//            }
        }

        if (quote_double == 1 || quote_single == 1)
        {
//            printf("now quote_double = %d", quote_double);
//            fflush(stdout);
//            if (tmp_char == '|')
//            {
//                tmp_line[loc] = '*';
//                loc++;
//                tmp_line[loc] = '*';
//                loc++;
//                tmp_line[loc] = '|';
//                loc++;
//            }
//            else
//            {
                if (tmp_char == '|')
                {
                    tmp_line[loc] = '*';
                    loc++;
//                    pipe_in_quote = 1;
                }
                tmp_line[loc] = (char)tmp_char;
                loc++;
//            }
        }

        if (shell_inloop == -1)
        {
            reading = 0;
            return tmp_line;
        }

        if (tmp_char == EOF)
        {
            printf("exit\n");
            exit(0);
        }

        // Check if the input is over
        if (tmp_char == '\n' && (quote_double == 1 || quote_single == 1
                                 || tmp_line[loc-1] == '>' ||
                                  tmp_line[loc-2] == '>' ||
                                  tmp_line[loc-1] == '<' ||
                                  tmp_line[loc-2] == '<' ||
                                  (tmp_line[loc-1] == '|')))
        {
            if (tmp_line[loc-1] == '>')
            {
                tmp_line[loc] = ' ';
                loc++;
            }
            printf("> ");
            fflush(stdout);
        }

        else if (tmp_char == '\n' && quote_double == 0 && quote_single == 0)
        {
            tmp_line[loc] = '\0';
            reading = 0;
//            pipe_in_quote = 0;
            return tmp_line;
        }


        // Check redirection sign
        else if (quote_double == 0 && quote_single == 0)
        {
            if (tmp_char == '<')
            {
                tmp_line[loc] = ' ';
                loc++;
                tmp_line[loc] = (char)tmp_char;
                loc++;
                tmp_line[loc] = ' ';
                loc++;
            }
            else if (tmp_char != ' ' && tmp_line[loc-1] == '>' && tmp_char != '>')
            {
                tmp_line[loc] = ' ';
                loc++;
                tmp_line[loc] = (char)tmp_char;
                loc++;
            }
            else if (tmp_char == '>' && tmp_line[loc-1] != '>' && tmp_line[loc-1] != ' ')
            {
                tmp_line[loc] = ' ';
                loc++;
                tmp_line[loc] = (char)tmp_char;
                loc++;
            }
            else if (tmp_line[loc-1] == '>' && tmp_char == '>')
            {
                tmp_line[loc] = (char)tmp_char;
                loc++;
                tmp_line[loc] = ' ';
                loc++;
            }

                // Check pipe sign
            else if (tmp_char == '|' && tmp_line[loc-1] != ' ')
            {
                tmp_line[loc] = ' ';
                loc++;
                tmp_line[loc] = (char)tmp_char;
                loc++;
            }
            else if (tmp_char != ' ' && tmp_line[loc-1] == '|')
            {
                tmp_line[loc] = ' ';
                loc++;
                tmp_line[loc] = (char)tmp_char;
                loc++;
            }
            else
            {
                tmp_line[loc] = (char)tmp_char;
                loc++;
            }
        }
//        else
//        {
//            tmp_line[loc] = (char)tmp_char;
//            loc++;
//        }
    }
}

int mumsh_parse_content(content_array_t *content_array, char* line_content)
{
    int loc = 0;
    char* tmp_arg;

    // Remove space from line_content
    const char* delim = " ";
    tmp_arg = strtok(line_content, delim);
    content_array->array[loc] = tmp_arg;
    loc++;
    while (tmp_arg != NULL)
    {
        tmp_arg = strtok(NULL, delim);
        content_array->array[loc] = tmp_arg;
        loc++;
    }

    // Add NULL to the end of commands array
    content_array->array[loc] = NULL;
    content_array->element_num = loc-1;
    return 0;
}

int mumsh_parse_args(content_array_t *content_array, pipe_t *complete_pipes)
{
    int loc = 0;
    complete_pipes->pipes_num = 0;
    mumsh_init_pipe(&complete_pipes->commands[loc]);
    while (loc < content_array->element_num)
    {
        if (!strcmp(content_array->array[loc], "|"))
        {
            complete_pipes->commands[complete_pipes->pipes_num].argv[complete_pipes->commands[complete_pipes->pipes_num].argc] = NULL;
            complete_pipes->pipes_num++;
            mumsh_init_pipe(&complete_pipes->commands[loc]);
        }
        else if (!strcmp(content_array->array[loc], "<"))
        {
            if (complete_pipes->commands[complete_pipes->pipes_num].input == 2)
            {
                perror("Input redirection error, multi redirection.\n");
                return -1;
            }
            if (loc + 1 < content_array->element_num)
            {
                complete_pipes->commands[complete_pipes->pipes_num].input = 2;
                loc++;
                complete_pipes->commands[complete_pipes->pipes_num].f_input = content_array->array[loc];
            }
            else
            {
                perror("Input redirection error, no input file.\n");
                return -1;
            }
        }
        else if (!strcmp(content_array->array[loc], ">"))
        {
            if (complete_pipes->commands[complete_pipes->pipes_num].output == 2
            || complete_pipes->commands[complete_pipes->pipes_num].output == 3)
            {
                printf("Output redirection error, multiple redirection\n");
                return -1;
            }
            if (loc + 1 < content_array->element_num)
            {
                complete_pipes->commands[complete_pipes->pipes_num].output = 3;
                loc++;
                complete_pipes->commands[complete_pipes->pipes_num].f_output = content_array->array[loc];
            }
            else
            {
                perror("Output redirection error, no output file.\n");
                return -1;
            }
        }
        else if (!strcmp(content_array->array[loc], ">>"))
        {
            if (complete_pipes->commands[complete_pipes->pipes_num].output == 2
                || complete_pipes->commands[complete_pipes->pipes_num].output == 3)
            {
                printf("Append output redirection error, multiple redirection\n");
                return -1;
            }
            if (loc + 1 < content_array->element_num)
            {
                complete_pipes->commands[complete_pipes->pipes_num].output = 2;
                loc++;
                complete_pipes->commands[complete_pipes->pipes_num].f_output = content_array->array[loc];
            }
            else
            {
                perror("Append output redirection error, no output file.\n");
                return -1;
            }
        }
        else
        {
            char *tmp = strstr(content_array->array[loc], "*|");
            if (tmp != NULL)
            {
//                printf("here");
                int i;
                for (i = 0; tmp[i] != '\0'; ++i) {
                    tmp[i] = tmp[i+1];
                }
                tmp[i] = '\0';
            }
            complete_pipes->commands[complete_pipes->pipes_num].argv[complete_pipes->commands[complete_pipes->pipes_num].argc]
            = content_array->array[loc];
            complete_pipes->commands[complete_pipes->pipes_num].argc++;
        }
        loc++;
    }
    complete_pipes->commands[complete_pipes->pipes_num].argv[complete_pipes->commands[complete_pipes->pipes_num].argc] = NULL;
    complete_pipes->pipes_num++;

//    for (int i = 0; i < complete_pipes->pipes_num; ++i)
//    {
//        for (int j = 0; j < complete_pipes->commands[i].argc; ++j) {
//            int quote = mumsh_parse_quote(complete_pipes->commands[i].argv[j]);
//        }
//    }
    return 0;
}

//int mumsh_parse_quote(char* arg)
//{
//    int first = 0;
////    int second = 0;
//    int quote = 0;
//    for (int i = 0; arg[i] != '\0'; ++i) {
//        if (arg[i] == '\"')
//        {
//            if (first == 0)
//            {
//                mumsh_parse_eliminate_quote(arg, i);
//                first = 1;
//                quote = 1;
//            }
//            if (first == 1)
//            {
//                mumsh_parse_eliminate_quote(arg, i);
//                first = 0;
//                quote = 0;
//            }
//        }
//    }
//    return quote;
//}

void mumsh_parse_eliminate_quote(char* arg, int loc)
{
    int i = loc;
    for ( ; arg[i] != '\0'; ++i) {
        arg[i] = arg[i+1];
    }
    arg[i] = '\0';
}

void mumsh_init_array(content_array_t *content_array)
{
    content_array->element_num = 0;
    for (int i = 0; i < ARG_SIZE; ++i)
    {
        content_array->array[i] = (char*)malloc(sizeof(char) * 8);
    }
}

void mumsh_init_pipe(single_command_t *command)
{
    command->input = 1;
    command->output = 1;
    command->argc = 0;
}

//void mumsh_free_array(content_array_t *content_array)
//{
//    for (int i = 0; i < content_array->element_num; ++i)
//    {
//        free(content_array->array[i]);
//    }
//}
//
//void mumsh_free_pipe(pipe_t *complete_pipes)
//{
//    for (int i = 0; i < complete_pipes->pipes_num; ++i)
//    {
//        for (int j = 0; j < complete_pipes->commands[i].argc; ++j)
//        {
//            free(complete_pipes->commands[i].argv[j]);
//        }
//        free(complete_pipes->commands[i].f_input);
//        free(complete_pipes->commands[i].f_output);
//    }
//}

//void mumsh_init_complete_pipe(pipe_t *complete_pipes)
//{
//    complete_pipes->pipes_num = 0;
//}
