// NAME - commands.case
//
// DESCRIPTION: commands.c parses the commands from smash into tokens and
//    decides what actions to take based on the input
//
// AUTHOR: Adam Berridge

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "smash.h"
#include "history.h"
#include <sys/types.h>
#include <sys/wait.h>

extern int errno;

void executeCommand(char *str){
  /*create variables*/
  const int MAX = 4096;
  int i = 0, exitStatus;
  char *args[MAX];
  char cmd[MAX];

  memset(cmd, 0, MAX); //zero out memory in cmd
  strncpy(cmd, str, strlen(str)); //cmd to be passed as arg to history.c methods

  for (int i = 0; i <= MAX; i++) {  //clear out args
    args[i] = NULL;
  }

  char *point = strtok(str, " "); //create first token

  while (point != NULL) {   //create remaining tokens from str
    args[i] = point;
    i++;
    point = strtok(NULL, " ");
  }
  args[i] = NULL;  //null terminate args array

/*
 if statements to identify the command and its arguments
*/
  if (args[0] == NULL) {
    return;
  }

  if (strncmp(args[0], "exit", 4096) == 0) {  //if command is exit
    clear_history();
    exit (0);
  }

  if (strncmp(args[0], "cd", MAX) == 0) {  //if command is cd
    if (args[1] == NULL) {  //if nothing entered
      return;
    }
    errno = 0;
    chdir(args[1]);
    if (errno != 0) {   //print error encountered when changing directories
      fprintf(stderr, "%s: %s\n", args[1], strerror(errno));
      /*include in history*/
      firstSequenceNumber++;
      exitStatus = 1;
      add_history(cmd, exitStatus);
    }
    else{               //print name of new directory location
      fprintf(stderr, "%s\n", args[1]);
      /*include in history*/
      firstSequenceNumber++;
      exitStatus = 0;
      add_history(cmd, exitStatus);
    }
  }
  else if (strncmp(args[0], "history", MAX) == 0) {  //if command is history
    /*include in history*/
    firstSequenceNumber++;
    exitStatus = 0;
    add_history(cmd, exitStatus);

    print_history(firstSequenceNumber);
  }
  else{     //if command is external or unkown
    int retVal = executeExternalCommand(str, args);

    /*include in history*/
    firstSequenceNumber++;
    exitStatus = retVal;
    add_history(cmd, exitStatus);
  }
  return;
}


/*
executeExternalCommand handles external and unrecognized commands passed
from smash
*/
int executeExternalCommand(char *str, char **args){
  int status;
  int retVal = fork();

  if(retVal < 0){  //if error
    perror("Unable to execute external command");
    status = -1;
  }
  else if(retVal > 0){ //if parent
    wait(&status);
  }
  else if(retVal == 0){  //if child
    execvp(args[0], args);
    char *val = args[0];
    perror(val);
    clear_history();
    _exit(3);
  }
  if (WIFEXITED(status)) {
    status = WEXITSTATUS(status);
  }
  return status;
}
