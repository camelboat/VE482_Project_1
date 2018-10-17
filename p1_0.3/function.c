#include "function.h"
#include "read_parse.h"

int shell_inloop;
fork_t shell_fork;

void mumsh_loop(fork_t *shell_fork, int *shell_inloop)
{
    char *line_content;
//    char **line_args;
    bool active = 1;
    extern int reading;
    while( 1 )
    {
        printf("mumsh $ ");
        fflush(stdout);
        line_content = mumsh_read_line();

//        printf("finish reading");

        if (line_content[0] == '\0' || *shell_inloop != 0)
        {
            *shell_inloop = 0;
            continue;
        }
        else
        {
            content_array_t content_array;
            mumsh_init_array(&content_array);
            if (mumsh_parse_content(&content_array, line_content) != 0)
            {
                perror("Error, failed to parse commands");
            }

            pipe_t complete_pipes;
            if (mumsh_parse_args(&content_array, &complete_pipes) != 0)
            {
                perror("Error, failed to parse commands' arguments");
            }


//            print_test(complete_pipes.commands[0].argv);
//            print_test_pipes(&complete_pipes);



//            if ()
            *shell_inloop = 1;
            if (reading == 0)
            {
                if (mumsh_execute_pipes(&complete_pipes, &active, shell_fork) != 0)
                {
                    active = 0;
                }
                *shell_inloop = 0;
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

void print_test(char** line_args)
{
    for (int i = 0; line_args[i] != NULL; ++i)
    {
        printf("%s ", line_args[i]);
    }
    printf("\n");
}

int mumsh_execute_pipes(pipe_t *complete_pipes, bool *active, fork_t *shell_fork)
{
//    int pid;
    int input = 0;
    int fd_pipe[2];
    shell_fork->forks_num = 0;
    int loc = 0;

    for (; loc < complete_pipes->pipes_num - 1; loc++) {
        *active = 0;
        if (!strcmp(complete_pipes->commands[loc].argv[0], "exit"))
        {
            *active = 1;
            return 0;
        }

        if (!strcmp(complete_pipes->commands[loc].argv[0], "cd"))
        {
            // write code for command cd
            int tmp = mumsh_funcion_cd(&complete_pipes->commands[loc]);
            if (tmp < 0)
            {
                perror("Error, failed to change working directory");
            }
            return 0;
        }

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

    *active = true;
    if (!strcmp(complete_pipes->commands[complete_pipes->pipes_num-1].argv[0], "exit"))
    {
        *active = false;
        return 0;
    }

    if (!strcmp(complete_pipes->commands[complete_pipes->pipes_num-1].argv[0], "cd"))
    {
        int tmp = mumsh_funcion_cd(&complete_pipes->commands[complete_pipes->pipes_num-1]);
        if (tmp < 0)
        {
            perror("Error, failed to change working directory");
        }
        return 0;
    }
    int tmp = mumsh_execute_single_command(&(complete_pipes->commands[complete_pipes->pipes_num-1]),
            input, 1, &(shell_fork->pid[shell_fork->forks_num]), true);
    shell_fork->forks_num++;
    if (tmp != 0)
    {
        return tmp;
    }

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

    int fd_input;
    int fd_output;
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
            fd_input = open(single_command->f_input, O_RDONLY);
            if (fd_input > 0)
            {
                dup2(fd_input, STDIN_FILENO);
                input_flag = 1;
            }
            else
            {
                printf("%s: No such file or directory\n", single_command->f_input);
//                perror("Error, failed to open file");
                return 0;
            }
        }
        if (single_command->output == 3)
        {
            fd_output = open(single_command->f_output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd_output > 0)
            {
                dup2(fd_output, STDOUT_FILENO);
                output_flag = 1;
            }
            else
            {
                printf("%s: No such file or directory\n", single_command->f_output);
//                perror("Error, failed to open file");
                return 0;
            }
        }
        if (single_command->output == 2)
        {
            fd_output = open(single_command->f_output, O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (fd_output > 0)
            {
                dup2(fd_output, STDOUT_FILENO);
                output_flag = 1;
            }
            else
            {
                printf("%s: No such file or directory\n", single_command->f_output);
//                perror("Error, failed to open file");
                return 0;
            }
        }
        if (!strcmp(single_command->argv[0], "pwd"))
        {
            mumsh_function_pwd();
            exit(0);
        }
        else if (execvp(single_command->argv[0], single_command->argv) < 0)
        {
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
        if (finish)
        {
            waitpid(pid, &loc, WUNTRACED);
//            while(wait(&loc) != pid);
            return 0;
        }
    }
    return 0;
//    return 0;
}

void print_test_pipes(pipe_t *complete_pipes)
{
    for (int i = 0; i < complete_pipes->pipes_num; ++i)
    {
        for (int j = 0; j < complete_pipes->commands[i].argc; ++j)
        {
            printf("command: %s\n", complete_pipes->commands[i].argv[j]);
//            printf("%c\n", complete_pipes->commands[i].argv[j][2]);
        }
        printf("file input: %s\n", complete_pipes->commands[i].f_input);
        printf("file ouptut: %s\n", complete_pipes->commands[i].f_output);
        printf("\n");
    }
}

int mumsh_funcion_cd(single_command_t *single_command)
{
    return chdir(single_command->argv[1]);
}

int mumsh_function_pwd()
{
    char buf[256];
    getcwd(buf, sizeof(buf));
    printf("%s\n", buf);
    return 0;
}


