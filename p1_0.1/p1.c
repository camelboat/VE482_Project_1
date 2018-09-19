#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void mumsh_loop();
char* mumsh_read_line();
char** mumsh_read_args(char* line_content);
int mumsh_do_task(char** line_args);

int main()
{
    mumsh_loop();
    return 0;
}

void mumsh_loop(){
    char *line_content;
    char **line_args;
    int active = 1;

    while( active )
    {
        printf("mumsh $");
        line_content = mumsh_read_line();
        line_args = mumsh_read_args(line_content);
        active = mumsh_do_task(line_args);
    }
}

char* mumsh_read_line()
{

}

char** mumsh_read_args(char* line_content)
{

}

int mumsh_do_task(char** line_args)
{

}
