//
// Created by Changxu_Luo on 18-9-19.
//

#include "function.h"
#include "read_parse.h"

/*
 * SIGINT signal handler, differentiate the conditions
 * inside the shell loop and outside the shell loop
 */
void mumsh_sigint_handler();

void mumsh_sigint_handler()
{
    extern int shell_inloop;
    if (shell_inloop == 1)
    {
        printf("\n");
        shell_inloop = -1;
    }
    else
    {
        printf("\n");
        shell_inloop = -1;
        return;
    }

}

int main()
{
    extern fork_t shell_fork;
    extern int shell_inloop;
    struct sigaction sa;
    sa.sa_handler = mumsh_sigint_handler;
    sa.sa_flags = 0;
    shell_inloop = 0;

    // Detect the asynchrony SIGTINT signal during
    // the whole program running
    sigaction(SIGINT, &sa, NULL);

    // Enter the main shell loop
    mumsh_loop(&shell_fork, &shell_inloop);
    return 0;
}
