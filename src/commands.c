#define _BSD_SOURCE 1
#define _SVID_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "commands.h"
#include "utils.h"

int do_bg(int argc, char** argv){

  int pid;
  int count = 0;
  int status;
  char** argv2 = malloc(8192); //to get rid of &
  char* PATH[5]; //EnvPATH

  for(int i=0; i<5;i++){
    PATH[i] = malloc(256);
  }
  for(int i = 0; i<argc-1; i++){//argc-1
    argv2[i] = malloc(512);
    strcpy(argv2[i], argv[i]);
  }

  
  if(strncmp(argv2[0], "/", 1))//error could occur
  {
    strcpy(PATH[0], "/usr/local/bin/");
    strcpy(PATH[1], "/usr/bin/");
    strcpy(PATH[2], "/bin/");
    strcpy(PATH[3], "/usr/sbin/");
    strcpy(PATH[4], "/sbin/");
  }
  for(int i = 0; i<5;i++){
    strcat(PATH[i], argv[0]);
    strcpy(argv2[0], PATH[i]);
    
    pid = fork();
    if(pid == 0){
      execv(argv2[0], argv2);     
      exit(EXIT_FAILURE);
    }
/*    waitpid(0, &status, WNOHANG); 
    if(status)
      break;*/

    free(PATH[i]);
  }
  free(argv2);
  
  return pid;
}
int do_launch(int argc, char** argv){

//  char curdir[PATH_MAX];
//  getcwd(curdir, PATH_MAX);
//  char* PATH[5] = {"/usr/local/bin/", "/usr/bin/",
//                 "/bin/", "/usr/sin/", "/sbin/"};
  int status = 0;
 //three different cases
 //1. absolute path input = input
  if(fork()==0){
    execv(argv[0], argv);
    exit(1);
  }
  
  wait(&status);

 //2. relative path input = currentDIR/input
 //3. resolved path input = PATH[i]/input
//  if(status)
//    return 1;  
  if(status)
    return -1;
  return 0;
 
}
int do_launch_resol(char* buf, int argc, char** argv){

  char* PATH[5];

  for(int i=0;i<5;i++){
    PATH[i] = malloc(256);
  }
  
  strcpy(PATH[0], "/usr/local/bin/");
  strcpy(PATH[1], "/usr/bin/");
  strcpy(PATH[2], "/bin/");
  strcpy(PATH[3], "/usr/sbin/");
  strcpy(PATH[4], "/sbin/");
  
  int status = 0;
  int pid;
  int exitVal;
  int exitStat;
  int count;
  for(int i=0; i<5;i++){
    strcat(PATH[i], buf);
  }
  
  for(count=0; count<5; count++){
    mysh_parse_command(PATH[count], &argc, &argv);
    if(fork()==0){
      execv(argv[0], argv);
      exit(EXIT_FAILURE);
    }
    
    pid = wait(&status);
    exitVal = WEXITSTATUS(status);
    exitStat = WIFEXITED(status);

    
    if(!exitVal)
      break;
  }
  for(int i = 0; i<5 ; i++){
    free(PATH[i]);
  }
 
  if(count == 5)
    return -1;

  return 0;
}
int do_ls(int argc, char** argv) {
  if (!validate_ls_argv(argc, argv))
    return -1;

  struct dirent **dirs;
  int dirnum = scandir(".", &dirs, NULL, alphasort);
  
  if (dirnum == 0) {
    printf("Empty directory.\n");
    return -1;
  }

  for(int i = 0; i<dirnum; i++)
  {
    if (!strcmp(dirs[i]->d_name, ".") || 
        !strcmp(dirs[i]->d_name, "..") ||
        !strcmp(dirs[i]->d_name, ".git") || 
        !strcmp(dirs[i]->d_name, ".gitignore"))
      continue;

    struct stat fstat;
    lstat(dirs[i]->d_name, &fstat);
    if((fstat.st_mode & S_IFDIR) == S_IFDIR)
      printf("\033[1;94m%s  \033[0m", dirs[i]->d_name);
    else if((fstat.st_mode & S_IXUSR) == S_IXUSR)
      printf("\033[1;92m%s  \033[0m", dirs[i]->d_name); 
    else printf("\033[0;97m%s  \033[0m", dirs[i]->d_name);
   }
  
  printf("\n");
  return 0;
}
int do_cd(int argc, char** argv) {
  if (validate_cd_argv(argc, argv) == 0)
    return -1;
  
  if (validate_cd_argv(argc, argv) == 2)
    return 0;
 
  if (chdir(argv[1]) == -1)
    return -1;

  return 0;
}

int do_pwd(int argc, char** argv) {
  if (!validate_pwd_argv(argc, argv))
    return -1;

  char curdir[PATH_MAX];

  if (getcwd(curdir, PATH_MAX) == NULL)
    return -1;

  printf("%s\n", curdir);

  return 0;
}

int validate_cd_argv(int argc, char** argv) {

  if (strcmp(argv[0], "cd") != 0) return 0;

  struct stat buf;
  stat(argv[1], &buf);

  if (!S_ISDIR(buf.st_mode)) return 0;

  return 1;
}

int validate_pwd_argv(int argc, char** argv) {
  if (argc != 1) return 0;
  if (strcmp(argv[0], "pwd") != 0) return 0;

  return 1;
}
int validate_ls_argv(int argc, char** argv) {

  if (strcmp(argv[0], "ls") != 0) return 0;

  return 1;
}
