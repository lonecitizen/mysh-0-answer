//do_bg keeps executed when ls / & typed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "commands.h"
#include "utils.h"
static void sig_chld(int signo);
static void release_argv(int argc, char*** argv);
int bgPID = 0;

int main()
{
  char buf[8096];
  int argc;
  char** argv;
  int status;
  
  
//  signal(SIGCHLD, sig_chld);
    
    
 /*  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = SA_NODEFER | SA_NOCLDWAIT;
  sigemptyset(&sa.sa_mask);
  sa.sa_restorer = NULL;
  sigaction(SIGCHLD, &sa, NULL);
*/

  while (1) {
    //signal(SIGCHLD, sig_chld);
   // printf("\033[1;92mJino's Shell $ \033[0m");
    fgets(buf, 8096, stdin);
    signal(SIGCHLD, sig_chld);
    mysh_parse_command(buf, &argc, &argv);

//    signal(SIGCHLD, sig_chld);
//    printf("signal returns %d\n", status);
    
    if (strcmp(argv[0], "") == 0) {
      goto release_and_continue;
    } else if (strcmp(argv[0], "cd") == 0 ) {
      if (do_cd(argc, argv)) {
        fprintf(stderr, "cd: Invalid arguments\n");
      }
/*    } else if (strcmp(argv[0], "pwd") == 0) {
      if (do_pwd(argc, argv)) {
        fprintf(stderr, "pwd: Invalid arguments\n");
      }
    } else if (strcmp(argv[0], "ls") == 0 && argc == 1) {
      if (do_ls(argc, argv)) {
        fprintf(stderr, "ls: Invalid arguments\n");
      }*/
    } else if (strcmp(argv[0], "exit") == 0) {
      goto release_and_exit;
    } else if (strcmp(argv[argc-1], "&") == 0) {
      printf("before bg\n");
      bgPID = do_bg(argc, argv);
      printf("%d\n", bgPID);
//      signal(SIGCHLD, sig_chld);
      //goto release_and_continue;
    } else if (strcmp(argv[0], "fg") == 0){
      if(bgPID){
        printf("%d running\n", bgPID);
        wait(&status);
        bgPID = 0;
      }
    } else {
      if(do_launch(argc, argv)){
        if(do_launch_resol(buf, argc, argv)){
          fprintf(stderr, "Process creation failed\n");
        }
      }
      //goto release_and_continue;
    }
//    signal(SIGCHLD, sig_chld);
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
  
  while((pid = waitpid(-1, &status, WNOHANG))>0)
  {
    if(pid == bgPID)
      printf("%d done \n", pid);
    else kill(-1, signo);
  } 
   return;
}
