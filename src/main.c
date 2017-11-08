#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wait.h> //WNOHANG

#include "commands.h"
#include "built_in.h"
#include "utils.h"
#include "signal_handlers.h"

void bg_handler()
{
	pid_t pid;
	
	while((pid=waitpid(-1, NULL, WNOHANG))>0){
		fprintf(stderr,"CHILD EXIT %d\n",pid);
	}
}

int main()
{
  //Signal Handling
    catch_sigint(SIGINT);
    catch_sigtstp(SIGTSTP);  

  char buf[8096];

signal(SIGCHLD, bg_handler);

  while (1) {
printf("mysh :");//dd
    fgets(buf, 8096, stdin);

    struct single_command commands[512];
    int n_commands = 0;
    mysh_parse_command(buf, &n_commands, &commands);

    int ret = evaluate_command(n_commands, &commands);

    free_commands(n_commands, &commands);
    n_commands = 0;

    if (ret == 1) {
      break;
    }
  }

  return 0;
}
