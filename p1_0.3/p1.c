#include "function.h"
#include "read_parse.h"

//bool interrupt;

void mumsh_sigint_handler();

void mumsh_sigint_handler()
{
    extern int shell_inloop;
//    extern fork_t shell_fork;
    if (shell_inloop == 1)
    {
//        int child_status;
//        for (int i = 0; i < shell_fork.forks_num; ++i) {
//            waitpid(shell_fork.pid[shell_fork.forks_num], &child_status, WUNTRACED);
//        }
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
//    signal(SIGINT, mumsh_sigint_handler);
    sigaction(SIGINT, &sa, NULL);
    mumsh_loop(&shell_fork, &shell_inloop);
    return 0;
}
