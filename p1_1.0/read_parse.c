//
// Created by Changxu_Luo on 18-10-2.
//

#include "read_parse.h"
#include "function.h"

int reading;
int program_missing = 0;

char *mumsh_read_line(bool *syntax)
{
    // whether the program is reading from the input
    extern int reading;

    // whether the program is running inside the main loop
    extern int shell_inloop;

    // define the maximum line size
    size_t line_size = 1024;

    // temporary container for the line content
    char *tmp_line = (char*)malloc(line_size);
    int tmp_char;
    int loc = 0;

    // whether the line has half part of the doulbe quotation
    int quote_double = 0;

    // whether the line has half part of the single quotation
    int quote_single = 0;

    // begins reading from the input
    reading = 1;
    while(1) {

        // read from stdin char by char
        tmp_char = getchar();

            // solve the problem of double quotation
            if (tmp_char == '\"') {
                if (quote_single == 0) {
                    if (quote_double == 0) {
                        quote_double = 1;
                        continue;
                    }
                    if (quote_double == 1) {
                        quote_double = 0;
                        continue;
                    }
                }
            }

            // solve the problem of single quotation
            if (tmp_char == '\'') {
                if (quote_double == 0) {
                    if (quote_single == 0) {
                        quote_single = 1;
                        continue;
                    }
                    if (quote_single == 1) {
                        quote_single = 0;
                        continue;
                    }
                }
            }

            // when the line has half of the single or double
            // quotation, sign "|" should not be treated as
            // the pipe sign
            if (quote_double == 1 || quote_single == 1) {
                if (tmp_char == '|') {
                    tmp_line[loc] = '*';
                    loc++;
                }
                tmp_line[loc] = (char) tmp_char;
                loc++;
            }

            if (shell_inloop == -1) {
                reading = 0;
                return tmp_line;
            }

            if (tmp_char == EOF) {
                printf("exit\n");
                exit(0);
            }

            // Check if the process of input is over when the user input '\0'
            if (tmp_char == '\n' && (quote_double == 1 || quote_single == 1
                                     || tmp_line[loc - 1] == '>' ||
                                     tmp_line[loc - 2] == '>' ||
                                     tmp_line[loc - 1] == '<' ||
                                     tmp_line[loc - 2] == '<' ||
                                     (tmp_line[loc - 1] == '|' && tmp_line[loc - 2] != '*'))) {
                char back_char = tmp_line[loc - 1];
                if ((back_char == '<' || back_char == '>' || back_char == '|') && *syntax == 1) {
                    if ((tmp_line[loc - 3] == '>' || tmp_line[loc - 3] == '<' || tmp_line[loc - 3] == '|') &&
                        tmp_line[loc - 4] != '*') {
                        printf("syntax error near unexpected token `%c'\n", back_char);
                        *syntax = 0;
                        return tmp_line;
                    }
                }
                if (tmp_line[loc - 1] == '>') {
                    tmp_line[loc] = ' ';
                    loc++;
                }
                printf("> ");
                fflush(stdout);
            }
            // check syntax error
            else if (tmp_char == '\n' && quote_double == 0 && quote_single == 0) {
                char back_char = tmp_line[loc - 1];
                if ((back_char == '<' || back_char == '>' || back_char == '|') && *syntax == 1) {
                    if (tmp_line[loc - 3] == '>' || tmp_line[loc - 3] == '<' || tmp_line[loc - 3] == '|') {
                        printf("syntax error near unexpected token `%c'\n", tmp_char);
                        *syntax = 0;
                        return tmp_line;
                    }
                }
                tmp_line[loc] = '\0';
                reading = 0;
                return tmp_line;
            }

            // Check redirection sign
            else if (quote_double == 0 && quote_single == 0) {
                if (tmp_char == '<') {
                    tmp_line[loc] = ' ';
                    loc++;
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                    tmp_line[loc] = ' ';
                    loc++;
                } else if (tmp_char != ' ' && tmp_line[loc - 1] == '>' && tmp_char != '>') {
                    tmp_line[loc] = ' ';
                    loc++;
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                } else if (tmp_char == '>' && tmp_line[loc - 1] != '>' && tmp_line[loc - 1] != ' ') {
                    tmp_line[loc] = ' ';
                    loc++;
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                } else if (tmp_line[loc - 1] == '>' && tmp_char == '>') {
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                    tmp_line[loc] = ' ';
                    loc++;
                }

                // Check pipe sign
                else if (tmp_char == '|' && tmp_line[loc - 1] != ' ') {
                    tmp_line[loc] = ' ';
                    loc++;
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                } else if (tmp_char != ' ' && tmp_line[loc - 1] == '|') {
                    tmp_line[loc] = ' ';
                    loc++;
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                } else {
                    tmp_line[loc] = (char) tmp_char;
                    loc++;
                }
            }
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
    // start arguments parsing
    while (loc < content_array->element_num)
    {
        // distribute the arguments into different pipes
        if (!strcmp(content_array->array[loc], "|"))
        {
            complete_pipes->commands[complete_pipes->pipes_num].argv[complete_pipes->commands[complete_pipes->pipes_num].argc] = NULL;
            complete_pipes->pipes_num++;
            mumsh_init_pipe(&complete_pipes->commands[loc]);
        }
        // check the redirection sign
        else if (!strcmp(content_array->array[loc], "<"))
        {
            if (complete_pipes->commands[complete_pipes->pipes_num].input == 2)
            {
                printf("error: duplicated input redirection\n");
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
                printf("Input redirection error, no input file.\n");
                return -1;
            }
        }
        else if (!strcmp(content_array->array[loc], ">"))
        {
            if (complete_pipes->commands[complete_pipes->pipes_num].output == 2
            || complete_pipes->commands[complete_pipes->pipes_num].output == 3)
            {
                printf("error: duplicated output redirection\n");
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
                printf("error: duplicated output redirection\n");
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
            // check if the pipe sign | is inside the quotation signs
            char *tmp = strstr(content_array->array[loc], "*|");
            if (tmp != NULL)
            {
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

    // check whether there are any commands lacks the number of program
    for (int j = 0; j < complete_pipes->pipes_num; ++j) {
        if (complete_pipes->commands[j].argc == 0 )
        {
            program_missing = 1;
            printf("error: missing program\n");
            return -1;
        }
    }
    return 0;
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
