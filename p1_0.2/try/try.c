#include <stdio.h>

void mumsh_parse_eliminate_quote(char* arg, int loc);

int main()
{
    char arg[10];
    arg[0] = 'h';
    arg[1] = 'i';
    arg[2] = '\"';
    arg[3] = 't';
    arg[4] = 'h';

    mumsh_parse_eliminate_quote(arg, 2);

    printf("%s", arg);
}


void mumsh_parse_eliminate_quote(char* arg, int loc)
{
    int i = loc;
    for ( ; arg[i] != '\0'; ++i) {
        arg[i] = arg[i+1];
    }
    arg[i] = '\0';
}

