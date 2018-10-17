//
// Created by Changxu_Luo on 18-9-19.
//

#include "function.h"
#include "read_parse.h"

int shell_inloop;
fork_t shell_fork;
int job_num;
char **job_list[256];
back_job_t jobs;

void mumsh_loop(fork_t *shell_fork, int *shell_inloop)
{
    // contain the single line string
    char *line_content;

    // whether the shell should exit or not
    bool active = 1;

    // whether there are syntax errors
    bool syntax;

    // whether the program is reading from the input
    extern int reading;

    // background jobs numbers
    extern int job_num;
    job_num = 1;

    // background jobs data
    extern back_job_t jobs;

    // whether the program name is missed in the command
    int program_missing;

    // whether the program has background job in this turn
    int background = 0;

    // start the main loop
    while( 1 )
    {
        syntax = 1;
        program_missing = 0;
        if (background == 0)
        {
            printf("mumsh $ ");
            fflush(stdout);
        }
        line_content = mumsh_read_line(&syntax);

        // check the syntax error
        if (syntax)
        {
            for (int j = 2; line_content[j] != '\0'; ++j) {
                char tmp_char = line_content[j];
                char tmp_char_2 = line_content[j-2];
                char tmp_char_3 = line_content[j-3];
                if (tmp_char == '<' || tmp_char == '>' || tmp_char == '|')
                {
                    if ((tmp_char_3 == '>' || tmp_char_3 == '<'
                        || tmp_char_2 == '>' || tmp_char_2 == '<') && tmp_char_2 != '*')
                    {
                        printf("syntax error near unexpected token `%c'\n", tmp_char);
                        syntax = 0;
                    }
                }
            }
        }

        if (syntax)
        {
            // if the user enters nothing, keep going
            if (line_content[0] == '\0' || *shell_inloop != 0)
            {
                *shell_inloop = 0;
                continue;
            }

            else
            {
                // parse the content from the line string
                content_array_t content_array;
                mumsh_init_array(&content_array);
                if (mumsh_parse_content(&content_array, line_content) != 0)
                {
                    perror("Error, failed to parse commands");
                }

                // parse the argument from the elements array
                pipe_t complete_pipes;
                if (mumsh_parse_args(&content_array, &complete_pipes) != 0)
                {
                    continue;
                }

                if (!program_missing)
                {
                    // detect duplicated input and output redirection
                    for (int i = 0; i < complete_pipes.pipes_num; ++i) {
                        if (complete_pipes.commands[i].input != 1 && complete_pipes.commands[i].f_input != NULL && i != 0)
                        {
                            printf("error: duplicated input redirection\n");
                            free(line_content);
                            break;
                        }
                        if (complete_pipes.commands[i].output != 1 && complete_pipes.commands[i].f_output != NULL && i != complete_pipes.pipes_num-1)
                        {
                            printf("error: duplicated output redirection\n");
                            free(line_content);
                            break;
                        }
                    }
                }

            /*
            // debugging functions
            print_test(complete_pipes.commands[0].argv);
            print_test_pipes(&complete_pipes);
            */

                *shell_inloop = 1;
                if (reading == 0 && !program_missing)
                {
                    // detect and execute background commands
                    if (!strcmp(complete_pipes.commands[complete_pipes.pipes_num-1].argv[complete_pipes.commands[complete_pipes.pipes_num-1].argc-1], "&"))
                    {
                        single_command_t *single_command = &complete_pipes.commands[complete_pipes.pipes_num-1];

                        if (!strcmp(single_command->argv[single_command->argc-1], "&"))
                        {
                            background = 1;
                            printf("[%d] ", job_num);
                            for (int i = 0; i < complete_pipes.pipes_num; ++i) {
                                for (int j = 0 ; j < single_command->argc-1; ++j)
                                {
                                    printf("%s ", complete_pipes.commands[i].argv[j]);
                                }
                                if (i != complete_pipes.pipes_num-1)
                                {
                                    printf("| ");
                                }
                            }
                            printf("%s\n", single_command->argv[single_command->argc-1]);
                            jobs.commands[job_num] = complete_pipes.commands;
                            job_num++;
                            single_command->argv[single_command->argc-1] = NULL;
                            printf("mumsh $ ");
                            fflush(stdout);
                        }
                    }
                    else
                    {
                        background = 0;
                    }

                    // execute common commands
                    if (mumsh_execute_pipes(&complete_pipes, &active, shell_fork) != 0)
                    {
                        active = 0;
                    }
                    *shell_inloop = 0;

                    // if the shell is not active, free the memory and exit the processes
                    if (active == 0)
                    {
                        free(line_content);
                        printf("exit\n");
                        exit(0);
                    }
                }
            }
        }
    }
}


int mumsh_execute_pipes(pipe_t *complete_pipes, bool *active, fork_t *shell_fork)
{
    int input = 0;
    int fd_pipe[2];
    shell_fork->forks_num = 0;
    int loc = 0;

    for (; loc < complete_pipes->pipes_num - 1; loc++) {
        *active = 0;

        // detect and execute "exit" command
        if (!strcmp(complete_pipes->commands[loc].argv[0], "exit"))
        {
            *active = 1;
            return 0;
        }

        // detect and execute "cd" command
        if (!strcmp(complete_pipes->commands[loc].argv[0], "cd"))
        {
            int tmp = mumsh_function_cd(&complete_pipes->commands[loc]);
            if (tmp < 0)
            {
                printf("%s: No such file or directory\n", complete_pipes->commands[loc].argv[1]);
            }
            return 0;
        }

        // form the pipes
        pipe(fd_pipe);
        int tmp =  mumsh_execute_single_command(&(complete_pipes->commands[loc]),
                input, fd_pipe[1], &(shell_fork->pid[shell_fork->forks_num]), false);
        shell_fork->forks_num++;

        if (tmp != 0)
        {
            return tmp;
        }

        close(fd_pipe[1]);
        input = fd_pipe[0];
    }

    // detect and execute "exit" command
    *active = true;
    if (!strcmp(complete_pipes->commands[complete_pipes->pipes_num-1].argv[0], "exit"))
    {
        *active = false;
        return 0;
    }

    // detect and execute "cd" command
    if (!strcmp(complete_pipes->commands[complete_pipes->pipes_num-1].argv[0], "cd"))
    {
        int tmp = mumsh_function_cd(&complete_pipes->commands[complete_pipes->pipes_num-1]);
        if (tmp < 0)
        {
            printf("%s: No such file or directory\n", complete_pipes->commands[complete_pipes->pipes_num-1].argv[1]);
        }
        return 0;
    }

    // execute other system commands supported by bash
    int tmp = mumsh_execute_single_command(&(complete_pipes->commands[complete_pipes->pipes_num-1]),
            input, 1, &(shell_fork->pid[shell_fork->forks_num]), true);
    shell_fork->forks_num++;
    if (tmp != 0)
    {
        return tmp;
    }

    // wait for child processes to finish
    int child_status;
    for (int i = 0; i < shell_fork->forks_num; ++i) {
        waitpid(shell_fork->pid[shell_fork->forks_num], &child_status, WUNTRACED);
    }

    return 0;
}

int mumsh_execute_single_command(single_command_t *single_command,
        int input, int output, int *forkpid, bool finish)
{
    int pid;
    int loc;
    int background = 0;

    int fd_input;
    int fd_output;
    extern int job_num;

    bool input_flag = 0;
    bool output_flag = 0;
    pid = fork();
    *forkpid = pid;

    if (pid == 0)
    {
        if (input != 0)
        {
            dup2(input, STDIN_FILENO);
            close(input);
        }
        if (output != 1)
        {
            dup2(output, STDOUT_FILENO);
            close(output);
        }
        if (single_command->input == 2)
        {
//            access(single_command->f_input, O_RDONLY);

            fd_input = open(single_command->f_input, O_RDONLY);
            if (errno == EACCES || errno == EROFS)
            {
                printf("%s: Permission denied\n", single_command->f_input);
                exit(0);
            }
            if (fd_input > 0)
            {
                dup2(fd_input, STDIN_FILENO);
                input_flag = 1;
            }
            else
            {
                printf("%s: No such file or directory\n", single_command->f_input);
                exit(0);
            }
        }
        if (single_command->output == 3)
        {
            fd_output = open(single_command->f_output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (errno == EACCES || errno == EROFS)
            {
                printf("%s: Permission denied\n", single_command->f_output);
                exit(0);
            }
            if (fd_output > 0)
            {
                dup2(fd_output, STDOUT_FILENO);
                output_flag = 1;
            }
            else
            {
                printf("%s: No such file or directory\n", single_command->f_output);
                exit(0);
            }
        }
        if (single_command->output == 2)
        {
            fd_output = open(single_command->f_output, O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (errno == EACCES || errno == EROFS)
            {
                printf("%s: Permission denied\n", single_command->f_output);
                exit(0);
            }
            if (fd_output > 0)
            {
                dup2(fd_output, STDOUT_FILENO);
                output_flag = 1;
            }
            else
            {
                printf("%s: No such file or directory\n", single_command->f_output);
                exit(0);
            }
        }


        if (!strcmp(single_command->argv[0], "pwd"))
        {
            mumsh_function_pwd();
            exit(0);
        }

        if (!strcmp(single_command->argv[0], "jobs"))
        {
            mumsh_function_jobs();
        }

        else if (execvp(single_command->argv[0], single_command->argv) < 0)
        {
            job_num++;
            printf("%s: command not found\n", single_command->argv[0]);
            exit(0);
        }

        if (input_flag)
        {
            close(fd_input);
        }
        if (output_flag)
        {
            close(fd_output);
        }
    }
    else
    {
        if (finish || !background)
        {
            waitpid(pid, &loc, WUNTRACED);
//            while(wait(&loc) != pid);
            return 0;
        }
    }
    return 0;
//    return 0;
}

// Implementation for "cd" command
int mumsh_function_cd(single_command_t *single_command)
{
    return chdir(single_command->argv[1]);
}

// Implementation for "pwd" command
int mumsh_function_pwd()
{
    char buf[256];
    getcwd(buf, sizeof(buf));
    printf("%s\n", buf);
    return 0;
}

// Implementation for "jobs" command
int mumsh_function_jobs()
{
    extern back_job_t jobs;
    extern int job_num;
    for (int i = 0; i < job_num; ++i) {
        printf("[%d] ", i+1);
        for (int j = 0; j < jobs.commands[i]->argc ; ++j) {
            printf("%s ", jobs.commands[i]->argv[j]);
        }
    }
    return 0;
}


/*
 * Test functions
 */
/*
void print_test(char** line_args)
{
    for (int i = 0; line_args[i] != NULL; ++i)
    {
        printf("%s ", line_args[i]);
    }
    printf("\n");
}


void print_test_pipes(pipe_t *complete_pipes)
{
    for (int i = 0; i < complete_pipes->pipes_num; ++i)
    {
        for (int j = 0; j < complete_pipes->commands[i].argc; ++j)
        {
            printf("command: %s\n", complete_pipes->commands[i].argv[j]);
        }
        printf("file input: %s\n", complete_pipes->commands[i].f_input);
        printf("file ouptut: %s\n", complete_pipes->commands[i].f_output);
        printf("\n");
    }
}
*/
