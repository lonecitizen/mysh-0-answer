//do_bg keeps executed when ls / & typed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "commands.h"
#include "utils.h"


static void sig_chld(int signo);
static void release_argv(int argc, char*** argv);
static void sig_bg(int signo);
static void fg_sig(int signo);

int bgflag = 0;
int bgPID = 0;
int main()
{
  char buf[8096];
  int argc;
  char** argv;
  int status;  

  signal(SIGCHLD, sig_bg);
  signal(SIGUSR1, SIG_IGN);    

  while (1) {

    buf[0] = 0;
    fgets(buf, 8096, stdin);

    mysh_parse_command(buf, &argc, &argv);

    if (strcmp(argv[0], "") == 0) {
      goto release_and_continue;
    } else if (strcmp(argv[0], "cd") == 0 ) {
      if (do_cd(argc, argv)) {
        fprintf(stderr, "cd: Invalid arguments\n");
      }
/*    } else if (strcmp(argv[0], "pwd") == 0) {
      if (do_pwd(argc, argv)) {
        fprintf(stderr, "pwd: Invalid arguments\n");
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

  while((bgPID = waitpid(-1, &status, WNOHANG))>0)
  {
    if(WIFEXITED(status))
      printf("%d done\n", bgPID);
    bgflag = 0;
  }  
  return;
}
static void fg_sig(int signo){  
  printf("%d running\n", getpid());
  signal(SIGUSR1, SIG_DFL);
}
