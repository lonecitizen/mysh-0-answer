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

#include "commands.h"

int do_launch(int argc, char** argv){

  char curdir[PATH_MAX];
  getcwd(curdir, PATH_MAX);
  int argc2;
  char** argv2;
  char* resolution = strcat(curdir, argv[0]);
  int temp;
  mysh_parse_command(resolution, &argc2, &argv2); 
 

  if(fork() == 0){
    temp = execv(argv[0], argv);
    if (temp == -1){
      if(fork() == 0){
        temp = execv(argv2[0], argv2);
        if(temp == -1)
          exit(EXIT_FAILURE);
      }
      exit(EXIT_FAILURE);
    }
  }
      /*perror("execv");
      exit(EXIT_FAILURE);
      return -1;*/

/*  wait();
  if(fork() == 0 && temp == -1){
    temp = execv(argv2[0], argv2);
    if (temp == -1)
      abort();
  }*//*if(fork() == 0){
    if(execv(argv2[0], argv2) == -1)
    {
      perror("execv");
      exit(EXIT_FAILURE);
      return -1;
    }
  }
  wait();*/
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
  if (!validate_cd_argv(argc, argv))
    return -1;

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
  if (argc != 2) return 0;
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
