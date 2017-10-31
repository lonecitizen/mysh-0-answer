#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "utils.h"

static void release_argv(int argc, char*** argv);

int main()
{
  char buf[8096];
  int argc;
  char** argv;

  while (1) {
    
    printf("\033[1;92mJino's Shell : \033[0m");
    fgets(buf, 8096, stdin);

    mysh_parse_command(buf, &argc, &argv);

    if (strcmp(argv[0], "") == 0) {
      goto release_and_continue;
    } else if (strcmp(argv[0], "cd") == 0) {
      if (do_cd(argc, argv)) {
        fprintf(stderr, "cd: Invalid arguments\n");
      }
    } else if (strcmp(argv[0], "pwd") == 0) {
      if (do_pwd(argc, argv)) {
        fprintf(stderr, "pwd: Invalid arguments\n");
      }
    } else if (strcmp(argv[0], "ls") == 0 && argc == 1) {
      if (do_ls(argc, argv)) {
        fprintf(stderr, "ls: Invalid arguments\n");
      }
    } else if (strcmp(argv[0], "exit") == 0) {
      goto release_and_exit;
    } else {
        if (do_launch(argc, argv)){
          fprintf(stderr, "Not executable.\n");
        } 
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
