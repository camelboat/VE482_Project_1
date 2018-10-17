#include "function.h"

void mumsh_loop()
{
    char *line_content;
    char ***line_pipes_args;
    char **line_args;
    int active = 1;
    int pipes_num = 1;

    while( active )
    {
        printf("mumsh $ ");
        fflush(stdout);
        line_content = mumsh_read_line();
        line_pipes_args = mumsh_parse_pipe(line_content);
        pipes_num = mumsh_count_pipes(line_pipes_args);
        print_test_pipe(line_pipes_args);


        line_args = mumsh_parse_args(line_content);
//        print_test(line_args);
        if (!strcmp(line_content, "exit"))
        {
            printf("exit");
            active = 0;
        }
        else
        {
//            active = mumsh_execute(line_args);
            active = mumsh_execute_pipes(line_pipes_args);
        }
        free(line_content);
        free(line_args);
    }
}

char *mumsh_read_line()
{
    size_t line_size = 1024;
    char *tmp_line = (char*)malloc(line_size);
    int tmp_char;
    int loc = 0;
    while(1)
    {
        tmp_char = getchar();
//        printf("%c", tmp_char);
//        if (tmp_char == '>' || tmp_char == '<')
//        {
//            tmp_line[loc] = ' ';
//            loc++;
//        }

        // Check redirection sign
        if (tmp_char == EOF || tmp_char == '\n')
        {
            tmp_line[loc] = '\0';
            return tmp_line;
        }
        if (tmp_char == '<')
        {
            tmp_line[loc] = ' ';
            loc++;
            tmp_line[loc] = tmp_char;
            loc++;
            tmp_line[loc] = ' ';
            loc++;
        }
        else if (tmp_char != ' ' && tmp_line[loc-1] == '>' && tmp_char != '>')
        {
            tmp_line[loc] = ' ';
            loc++;
            tmp_line[loc] = tmp_char;
            loc++;
//            tmp_line[loc] = ' ';
//            loc++;
        }
        else if (tmp_char == '>' && tmp_line[loc-1] != '>' && tmp_line[loc-1] != ' ')
        {
            tmp_line[loc] = ' ';
            loc++;
            tmp_line[loc] = tmp_char;
            loc++;
        }
        else if (tmp_line[loc-1] == '>' && tmp_char == '>')
        {
            tmp_line[loc] = tmp_char;
            loc++;
            tmp_line[loc] = ' ';
            loc++;
        }

            // Check pipe sign
        else if (tmp_char == "|" && tmp_line[loc-1] != ' ')
        {
            tmp_line[loc] = ' ';
            loc++;
            tmp_line[loc] = tmp_char;
            loc++;
        }
        else if (tmp_char != ' ' && tmp_line[loc-1] == "|")
        {
            tmp_line[loc] = ' ';
            loc++;
            tmp_line[loc] = tmp_char;
            loc++;
        }
        else
        {
            tmp_line[loc] = tmp_char;
            loc++;
        }
    }
}

char** mumsh_parse_args(char* line_content)
{
    int arg_num = 64;
    char** tmp_arg_all = (char**)malloc(arg_num * sizeof(char**));
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

char*** mumsh_parse_pipe(char* line_content)
{
    int arg_num = 256;
    int loc = 0;
    int tmp_loc = 0;
    int pipe_num = 0;
    printf("%s\n", line_content);
//    char*** tmp_pipe_arg = (char***)malloc(arg_num * sizeof(char*));
    char*** tmp_pipe_arg;
    while (line_content[loc] != '\0')
    {
        char *tmp = (char*)malloc(1024 * sizeof(char*));
        while (line_content[loc] != '|' && line_content[loc] != '\0')
        {
//            printf("loc %d \n", loc);
            tmp[tmp_loc] = line_content[loc];
            tmp_loc++;
            loc++;
        }
//        printf("loc out %d \n", loc);
        tmp[tmp_loc+1] = NULL;
        loc = loc+2;
        tmp_pipe_arg[pipe_num] = mumsh_parse_args(tmp);
        tmp_loc = 0;
//        free(tmp);
        pipe_num++;
    }
    return tmp_pipe_arg;
}

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
    int loc_tmp_2 = 0;
    int output_flag = 0;
    int input_flag = 0;
    int output_append_flag = 0;
    int out_fd;
    int in_fd;
    int out_append_fd;
    int fd_tmp;
    if (pid == -1)
    {
        perror("fork failed");
        return 0;
    }
    else if (pid == 0)
    {
        // Check output redirection
        for (int i = 0; line_args[i] != NULL ; ++i)
        {
            if (!strcmp(line_args[i], ">"))
            {
                loc_tmp = i;
                output_flag = 1;
//                line_args[loc_tmp] = NULL;
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
            fd_tmp = open(line_args[loc_tmp+1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
//            line_args[loc_tmp+1] = NULL;
            for (int j = loc_tmp; line_args[j] != NULL; ++j) {
                line_args[j] = line_args[j+2];
            }
            out_fd = dup2(1, fd_tmp);
        }

        if (!output_flag) {
            // Check appending output redirection
            loc_tmp = 0;
            for (int i = 0; line_args[i] != NULL; ++i) {
                if (!strcmp(line_args[i], ">>")) {
                    loc_tmp = i;
                    output_append_flag = 1;
//                    line_args[loc_tmp] = NULL;
                    break;
                }
            }
            if (output_append_flag) {
                if (line_args[loc_tmp + 1] == NULL) {
                    perror("output redirection error\n");
                    return 1;
                }
                close(1);
                fd_tmp = open(line_args[loc_tmp+1], O_WRONLY | O_CREAT | O_APPEND, 0777);
//                line_args[loc_tmp + 1] = NULL;
                for (int j = loc_tmp; line_args[j] != NULL; ++j) {
                    line_args[j] = line_args[j+2];
                }
                out_append_fd = dup2(1, fd_tmp);
            }
        }

        // Check input redirection
        for (int i = 0; line_args[i] != NULL; ++i)
        {
            if (!strcmp(line_args[i], "<"))
            {
                loc_tmp_2 = i;
                input_flag = 1;
//                line_args[loc_tmp_2] = NULL;
                break;
            }
        }
        if (input_flag)
        {
            if (line_args[loc_tmp_2+1] == NULL)
            {
                perror("input redirection error\n");
                return 1;
            }
            close(0);
            fd_tmp = open(line_args[loc_tmp_2+1], O_RDONLY);
//            line_args[loc_tmp_2+1] = NULL;
            for (int j = loc_tmp_2; line_args[j] != NULL; ++j) {
                line_args[j] = line_args[j+2];
            }
//            print_test(line_args);
            in_fd = dup2(0, fd_tmp);
        }
//        printf("%d\n%d\n%d\n", output_flag, output_append_flag, input_flag);
        execvp(line_args[0], line_args);
        if (output_flag)
        {
            close(1);
            dup2(out_fd, 1);
        }
        if (input_flag)
        {
            close(0);
            dup2(in_fd, 0);
        }
        if (output_append_flag)
        {
            close(1);
            dup2(out_append_fd, 1);
        }
        perror("execvp failed");
        return 0;
    } else {
        while(wait(&exitstatus) != pid);
        return 1;
    }
}

int mumsh_execute_pipes(char*** line_pipes_args)
{
    if (line_pipes_args[0][0] == NULL)
    {
        return 1;
    }
    else
    {
        int pipe_num = mumsh_count_pipes(line_pipes_args);
        int *fd_last = (int*)malloc(sizeof(int) * 2);
        int *fd_next = (int*)malloc(sizeof(int) * 2);
        for (int i = 0; i < pipe_num; ++i)
        {
            pipe(fd_next);
            if (i == pipe_num-1)
            {
                close(fd_next[0]);
                close(fd_next[1]);
                fd_next[0] = -1;
            }
            int *tmp = mumsh_start_single_pipe(line_pipes_args[0], fd_last, fd_next);
            fd_last[0] = tmp[0];
            fd_last[1] = tmp[1];
        }
        free(fd_last);
        free(fd_next);
    }
    return 1;
}

int* mumsh_start_single_pipe(char** line_args, int* fd_last, int* fd_next)
{
    int pid, exitstatus;
    pid = fork();
    int loc_tmp = 0;
    int loc_tmp_2 = 0;
    int output_flag = 0;
    int input_flag = 0;
    int output_append_flag = 0;
    int out_fd;
    int in_fd;
    int out_append_fd;
    int fd_tmp;
    if (fd_last != NULL)
    {
        dup2(fd_last[0], STDIN_FILENO);
        close(fd_last[1]);
    }
    if (fd_next[0] != -1)
    {
        dup2(fd_next[0], STDOUT_FILENO);
    }
    if (pid == -1)
    {
        perror("fork failed");
        return 0;
    }
    else if (pid == 0)
    {
//        // Check output redirection
//        for (int i = 0; line_args[i] != NULL ; ++i)
//        {
//            if (!strcmp(line_args[i], ">"))
//            {
//                loc_tmp = i;
//                output_flag = 1;
////                line_args[loc_tmp] = NULL;
//            }
//        }
//        if (output_flag)
//        {
//            if (line_args[loc_tmp+1] == NULL)
//            {
//                perror("output redirection error\n");
//                return 1;
//            }
//            close(1);
//            fd_tmp = open(line_args[loc_tmp+1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
////            line_args[loc_tmp+1] = NULL;
//            for (int j = loc_tmp; line_args[j] != NULL; ++j) {
//                line_args[j] = line_args[j+2];
//            }
//            out_fd = dup2(1, fd_tmp);
//        }
//
//        if (!output_flag) {
//            // Check appending output redirection
//            loc_tmp = 0;
//            for (int i = 0; line_args[i] != NULL; ++i) {
//                if (!strcmp(line_args[i], ">>")) {
//                    loc_tmp = i;
//                    output_append_flag = 1;
////                    line_args[loc_tmp] = NULL;
//                    break;
//                }
//            }
//            if (output_append_flag) {
//                if (line_args[loc_tmp + 1] == NULL) {
//                    perror("output redirection error\n");
//                    return 1;
//                }
//                close(1);
//                fd_tmp = open(line_args[loc_tmp+1], O_WRONLY | O_CREAT | O_APPEND, 0777);
////                line_args[loc_tmp + 1] = NULL;
//                for (int j = loc_tmp; line_args[j] != NULL; ++j) {
//                    line_args[j] = line_args[j+2];
//                }
//                out_append_fd = dup2(1, fd_tmp);
//            }
//        }
//
//        // Check input redirection
//        for (int i = 0; line_args[i] != NULL; ++i)
//        {
//            if (!strcmp(line_args[i], "<"))
//            {
//                loc_tmp_2 = i;
//                input_flag = 1;
////                line_args[loc_tmp_2] = NULL;
//                break;
//            }
//        }
//        if (input_flag)
//        {
//            if (line_args[loc_tmp_2+1] == NULL)
//            {
//                perror("input redirection error\n");
//                return 1;
//            }
//            close(0);
//            fd_tmp = open(line_args[loc_tmp_2+1], O_RDONLY);
////            line_args[loc_tmp_2+1] = NULL;
//            for (int j = loc_tmp_2; line_args[j] != NULL; ++j) {
//                line_args[j] = line_args[j+2];
//            }
////            print_test(line_args);
//            in_fd = dup2(0, fd_tmp);
//        }
//        printf("%d\n%d\n%d\n", output_flag, output_append_flag, input_flag);
        execvp(line_args[0], line_args);
//        if (output_flag)
//        {
//            close(1);
//            dup2(out_fd, 1);
//        }
//        if (input_flag)
//        {
//            close(0);
//            dup2(in_fd, 0);
//        }
//        if (output_append_flag)
//        {
//            close(1);
//            dup2(out_append_fd, 1);
//        }
        close(fd_last[0]);
        perror("execvp failed");
        return 0;
    } else {
        while(wait(&exitstatus) != pid);
        return fd_next;
    }
}

int mumsh_count_pipes(char*** line_pipes_args)
{
    int pipe_num = 0;
    while (line_pipes_args[pipe_num] != NULL)
    {
        pipe_num++;
    }
    return pipe_num;
}

void print_test(char** line_args)
{
    for (int i = 0; line_args[i] != NULL; ++i) {
        printf("%s ", line_args[i]);
    }
}

void print_test_pipe(char ***line_pipes_args)
{
    int pipe_num = 0;
    while (line_pipes_args[pipe_num] != NULL)
    {
        print_test(line_pipes_args[pipe_num]);
//        printf("here");
        printf("\n");
        pipe_num++;
    }
    printf("has %d pipes", pipe_num);
}