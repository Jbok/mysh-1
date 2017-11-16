#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>	//pid_t
#include <unistd.h>	//fork
#include <sys/wait.h>	//wait
#include <signal.h>

//IPC
#include <sys/socket.h> //socket
#include <sys/types.h>
#include <pthread.h> //thread
#include <netinet/in.h>//sockaddr_in storage size error
#include <sys/un.h>//sockaddr_un storage size error
#include <errno.h>
//UDS using TCP
#include <arpa/inet.h>

#include "commands.h"
#include "built_in.h"

#define FILE_SERVER "/tmp/test_server"

//thread function
void* thr_fn(void* argv){
	//bring command
	struct single_command* com=(struct single_command *)argv;
	
	//create server socket
	if(access("/tmp/test_server", F_OK)==0){
		unlink("/tmp/test_server");
	}
	int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket == -1){
		fprintf(stderr,"Error about server socket creation\n");
		exit(1);
	}
	//bind server socket , assign to kernel
	struct sockaddr_un server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, FILE_SERVER);

	//server_addr.sin_port = htons(4000);
	//server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==-1 ){
		fprintf(stdout,"%s\n",strerror(errno));		
		fprintf(stderr,"Error aboud bind()\n"); 
		exit(1);
	}
	//accept()
	while(1){
		if(listen(server_socket,5)==-1){
    	fprintf(stderr,"Error about listen()\n");
			continue;
		} 
		struct sockaddr_un client_addr;
		int client_socket;
		int client_addr_size;
		client_addr_size=sizeof(client_addr);
		client_socket=accept(server_socket, (struct sockaddr*)&client_addr,&client_addr_size);
		if( client_socket == -1){
			fprintf(stderr,"Error about connection to client server");
			continue;
		} 
		pid_t pid_server;
		pid_server=fork();
//printf("YES\n");
		if(pid_server==0){
			dup2(client_socket, 1);
			close(client_socket);
			execv(com->argv[0],com->argv);
		}
		close(client_socket);
		wait(0);		
		break;
	}
	pthread_exit(0);
}
void* bg_handler(void* argv){
	int stat;
	printf("%d\n",pid_bg);
	while( waitpid(pid_bg, &stat ,WNOHANG) == 0 ){ //0: WAIT_MYGRP, -1: WAIT_ANY	
	}
	fprintf(stdout,"%d done %s\n", pid_bg, command_bg);
	pthread_exit(0);	
}

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}



/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{

	if (n_commands > 0) {	
		struct single_command* com = (*commands);
		assert(com->argc != 0);
		int built_in_pos = is_built_in_command(com->argv[0]);
		if (built_in_pos != -1) {
			if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)){
				if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
					fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
					fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        	return -1;
				}
		}else if (strcmp(com->argv[0], "") == 0) {
				return 0;
			} else if (strcmp(com->argv[0], "exit") == 0) {
				return 1;
			} else {
			 	/////////////////////////////////////////////   
				//Inter-Process Communication and Threading//
				/////////////////////////////////////////////
				if( n_commands > 1){
					pthread_t thread;
					struct single_command* com1= com;
					struct single_command* com2= com+1;
 					/*
					char buf1[5][8096];
					char buf2[5][8096];
					for(int i=0;i<com->argc;i++){
						strcpy(buf1[i],com->argv[i]);
						com1->argv=buf1;
						com1->argc=i+1;
						printf("n_commands :%d, %s\n",n_commands,buf1[i]);
					}
					for(int i=0;i<(com+1)->argc;i++){
						strcpy(buf2[i],(com+1)->argv[i]);
						com2->argv=buf2;
						com2->argc=i+1;
	           printf("n_commands :%d, %s\n",n_commands,buf2[i]);
					}*/

					if(pthread_create(&thread, NULL, thr_fn,com1)!=0){//creat thread
						fprintf(stderr,"Error about thread occurs\n");
						exit(1);
					}
					/*wait for the thread to exit*/

					

//					pid_t pid_client;
//					pid_client=fork();
//					if (pid_client==0){
						//socket creation
						int client_socket;
						client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
						if(client_socket == -1){
							fprintf(stderr,"Error about socket creation\n");
							exit(1);
						}
						//server connection
						struct sockaddr_un server_addr;
						memset(&server_addr,0,sizeof(server_addr));
						server_addr.sun_family=AF_UNIX;
						strcpy(server_addr.sun_path,FILE_SERVER);
						//server_addr.sin_port=htons(4000);
						//server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
						while( connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
							//fprintf(stdout,"%s\n",strerror(errno));		
							//perror("error:");
							//fprintf(stderr,"Error about connecton\n");						
						} 
						//pthread_join(thread,NULL);
							//do command
						pid_t pid_client;
						pid_client=fork();
						if (pid_client==0){
							int fd_stdout=dup2(client_socket,STDIN_FILENO);
							if( fd_stdout == -1 ){
								fprintf(stderr, "Error about dup\n");
								exit(1);								
							} else{
								dup2(client_socket,0);
								close(client_socket);
								execv(com2->argv[0],com2->argv);	
							}
					close(client_socket);
					wait(0);			
					}
				}
	else{
// NewLine for Process Creation
	int Is_bg=-1; //Is background processing? Yes=1, No=-1


	if(!strcmp(com->argv[com->argc-1],"&")){
		com->argv[com->argc-1]=NULL;
		(com->argc)--; //delete '&'
		Is_bg=1;
	}

	//Path Resolution
	char path[5][512]={
		"/usr/local/bin/",
		"/bin/",
		"/usr/bin/",
		"/usr/sbin/",
		"/sbin/"};     

pid_t pid;
pid=fork();

      if(pid == -1){
				fprintf(stderr,"Error about fork occurs\n");
        exit(0);
      } else if (pid == 0){  //child
//Path Resolution
        if(execv(com->argv[0],com->argv)==-1){
					char *origin=com->argv[0];
					for(int i=0;i<5;i++){
						strcat(path[i],com->argv[0]);
						com->argv[0]=path[i];
						if(execv(com->argv[0],com->argv)==-1)
						com->argv[0]=origin;
				}
			}

			fprintf(stderr, "%s: command not found\n", com->argv[0]);
			exit(-1);

      } else{	//parent
				if(Is_bg==1){
					//Background Processing
					pid_bg=pid;
					pthread_t thread_bg;
					strcpy(command_bg,"");//initialize
					for(int i=0;i<com->argc;i++){
						strcat(command_bg,com->argv[i]);
						strcat(command_bg," ");
					}
					pthread_create(&thread_bg, NULL, bg_handler, NULL);
				}else{ //Not Background
					waitpid(pid,NULL,0);
					fflush(stdout); 
				}
			}
    }
  }

  return 0;
}
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
