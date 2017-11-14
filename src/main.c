#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <unistd.h>
#include <wait.h> //WNOHANG
#include <bits/sigaction.h>
#include <signal.h>
#include "commands.h"
#include "built_in.h"
#include "utils.h"
#include "signal_handlers.h"



void bg_handler(int signo)
{	
	pid_t pid;
	int stat;
	while( (pid= waitpid(-1, &stat ,WNOHANG))> 0 ){ //0: WAIT_MYGRP, -1: WAIT_ANY	
		fprintf(stdout,"%d done \n", pid);
	}
	signal(SIGCHLD,bg_handler);	
}


int main()
{
	char buf[8096];
	
    //	signal(SIGCHLD,bg_handler);		
	//Signal Handling
    	catch_sigint(SIGINT);
    	catch_sigtstp(SIGTSTP);  


  while (1) {
   
    fprintf(stdout,"mysh :");//dd
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
