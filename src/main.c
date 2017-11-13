#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "commands.h"
#include "utils.h"

static void server(char* temp);
static void client(char* temp2);
void* thread_listen();
static void sig_chld(int signo);
static void release_argv(int argc, char*** argv);
static void sig_bg(int signo);
static void fg_sig(int signo);
/*struct sockaddr_un{
  unsigned short int family;
  char path[PATH_MAX];
};*/

#define SOCK_PATH "tpf_unix_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"

int bgflag = 0;
int bgPID = 0;
int main()
{
  char buf[8096];
  int argc;
  char** argv;
  int status;  
  char* temp;
  char* temp2;
//  pthread_t th[5];
  signal(SIGCHLD, sig_bg);
  signal(SIGUSR1, SIG_IGN);    
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

  while (1) {

    buf[0] = 0;
    fgets(buf, 8096, stdin);

    mysh_parse_command(buf, &argc, &argv);

    for(int i=0; i<argc; i++){
      if (strcmp(argv[i], "|") == 0){
        temp = strtok(buf, "|");
        temp2 = strtok(NULL, "\n");
        
  //      pthread_create(&th[0], NULL, server, NULL);
        if(fork() == 0){
          server(temp);
          exit(2);
        } 

        if(fork() == 0){
          client(temp2);
          exit(2);          
        }
        //send function should be made
        
      }
    }
    
    if (strcmp(argv[0], "") == 0) {
      goto release_and_continue;
    } else if (strcmp(argv[0], "cd") == 0 ) {
      if (do_cd(argc, argv)) {
        fprintf(stderr, "cd: Invalid arguments\n");
      }
/*    } else if (strcmp(argv[0], "pwd") == 0) {
      if (do_pwd(argc, argv)) {
        fprintf(stderr, "pwd: Invalid arguments\n");
      if (do_pwd(argc, argv)) {
      if (do_pwd(argc, argv)) {
      }*/
    } else if (strcmp(argv[0], "ls2") == 0 && argc == 1) {
      if (do_ls(argc, argv)) {
        fprintf(stderr, "ls: Invalid arguments\n");
      }
    } else if (strcmp(argv[0], "exit") == 0) {
      goto release_and_exit;
    } else if (strcmp(argv[argc-1], "&") == 0 && bgflag != 1) {
      free(argv[argc-1]);
      argv[argc-1] = NULL;
      argc--;
      bgflag = 1;
      
      if(fork() == 0){
        signal(SIGUSR1, fg_sig);
        printf("%d\n", getpid());
        if(do_launch(argc, argv) == -1){
          fprintf(stderr, "%d failed\n", getpid());
          exit(1);
        }
        else{
          exit(2);
        }
      }
      
    } else if (strcmp(argv[0], "fg") == 0 && bgflag == 1){
      kill(0, SIGUSR1);
      wait(&status);
      bgflag = 0;
      
    } else if (bgflag==1 && argv[argc-1] == "&"){
      printf("Background Process already exits\n");  
    } else{
      if(do_launch(argc, argv) == -1)
        fprintf(stderr, "Process creation failed\n");
    }

release_and_continue:
    release_argv(argc, &argv);
    continue;
release_and_exit:
    release_argv(argc, &argv);
    break;
  }

  return 0;
}

static void release_argv(int argc, char*** argv) {
  for (int i = 0; i < argc; ++i) {
    free((*argv)[i]);
  }
  free(*argv);
  *argv = NULL;
}
static void sig_chld(int signo){
  int pid;
  int status;
  
  while((pid = waitpid(-1, &status, WNOHANG))>0);

  return;
}
static void sig_bg(int signo){
  int pid;
  int status;

  while((bgPID = waitpid(0, &status, WNOHANG))>0)
  {
    //if(WIFEXITED(status))
      printf("%d done\n", bgPID);
    bgflag = 0;
  }  
  return;
}
static void fg_sig(int signo){  
  printf("%d running\n", getpid());
  signal(SIGUSR1, SIG_DFL);
}
static void server(char* temp){

  char** argv_server;
  int argc_server;
  
  mysh_parse_command(temp, &argc_server, &argv_server);
  do_launch(argc_server, argv_server);//argument should be passed
  
  pthread_t server_thread;
  
  pthread_create(&server_thread, NULL, thread_listen, NULL);
   
  pthread_exit(NULL);
}
static void client(char* temp2){
  char** argv_client;
  int argc_client;
  int client_sock, rc, len;
  struct sockaddr_un server_sockaddr;
  struct sockaddr_un client_sockaddr;
  char* data;
  
  mysh_parse_command(temp2, &argc_client, &argv_client);

  memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
  memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
  memset(data, 0, 1000000);

  client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  
  client_sockaddr.sun_family =AF_UNIX;
  strcpy(client_sockaddr.sun_path, CLIENT_PATH);
  len = sizeof(client_sockaddr);

  unlink(CLIENT_PATH);
  bind(client_sock, (struct sockaddr *) &client_sockaddr, len);

  server_sockaddr.sun_family = AF_UNIX;
  strcpy(server_sockaddr.sun_path, SERVER_PATH);
  connect(client_sock, (struct sockaddr *) &server_sockaddr, len);

  recv(client_sock, data, sizeof(data), 0);
  fflush(stdin);
  fputs(data, stdin);
  do_launch(argc_client, argv_client);  

}
void* thread_listen(){

  int server_sock, client_sock, rc, len;
  struct sockaddr_un server_sockaddr;
  struct sockaddr_un client_sockaddr;
  char* data;
  int backlog = 10;

  memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
  memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
  memset(data, 0, 1000000);

  server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
 
  server_sockaddr.sun_family = AF_UNIX;
  strcpy(server_sockaddr.sun_path, SOCK_PATH);
  len = sizeof(server_sockaddr);

  unlink(SOCK_PATH);
  bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
  dup2(client_sock, 1); //not sure
  
  read(client_sock, data, 1000000);
  
  listen(server_sock, backlog);

  client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
  
  len = sizeof(client_sockaddr);
  getpeername(client_sock, (struct sockaddr *) &client_sockaddr, &len);

  send(client_sock, data, strlen(data), 0);

  close(client_sock);
  close(server_sock);

  //listen(server_sock, 10);
  pthread_exit(NULL);
}
