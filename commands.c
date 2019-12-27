// NAME - commands.c
//
// DESCRIPTION: commands.c parses the commands from smash into tokens and
//    decides what actions to take based on the input
//
// AUTHOR: Adam Berridge

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "smash.h"
#include "history.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


extern int errno;
//signal interrupt handler for Ctrl-C
//void sig_handler(){
  //fputs("^C\n", stderr);
//}

void executeCommand(char *str){

  //signal(SIGINT, sig_handler);

  const int MAX = 4096;
  int count = 0, exitStatus=0;
  char *args[MAX];
  char *container[MAX];
  char cmd[MAX];
  char *file;
  int newfd, num_elements, result=0, inchild=0, pipecount=0;
  int stdin=-2, stdout=-2, size=0, pipenum=0, temp=0;
  int inside=0;
  void shift_array(char **, int, int);
  void infoFiles();

  memset(cmd, 0, MAX); //zero out elements in cmd
  strncpy(cmd, str, strlen(str)); //cmd to be passed as arg to history.c methods

  for (int i = 0; i <= MAX; i++) {  //clear out container
    container[i] = NULL;
  }

  char *point = strtok(str, " "); //create first token

  while (point != NULL) {   //create remaining tokens from str
    container[count] = point;
    count++;
    point = strtok(NULL, " ");
  }
  container[count] = NULL;  //null terminate container array

  if (container[0] == NULL) { //if nothing entered
    return;
  }

  //count pipes
  for (size_t i = 0; i < count; i++) {
    if (strncmp(container[i], "|", strlen(container[i])) == 0) {
      pipecount++;
    }
  }
  //create pipes
  int fd[pipecount];
  for (size_t i = 0; i < pipecount; i++) {
    result = pipe(fd + i*2);

    if (result == -1) {
      perror("Could not pipe");
      //include in history
      exitStatus = -127;
      firstSequenceNumber++;
      add_history(cmd, exitStatus);
      return;
    }
  }
  int pids[pipecount+1];
  //initialize pids
  memset(pids, 0, pipecount+1 * sizeof(int));

  // parse container elements around pipes
  for (int i = 0; i < count; i++) {    //parse through args
    //if container[i] is not a pipe put it in args[i] until pipe or end of container reached
    if (strncmp(container[i], "|", strlen(container[i])) != 0){
      args[size] = container[i];
      size++;
      if (i != count-1) {
        continue;
      }
      args[i+1]=NULL;
    }
    //if container[i] is a pipe
    if (strncmp(container[i], "|", strlen(container[i])) == 0) {
      args[i] = NULL;
      //increment for the first pipe found
      if (!pipenum) {
        pipenum ++;
      }
    }

    //handle pipe
    if (pipenum) {
      inside++;

      pids[pipenum-1] = fork();

      if(pids[pipenum-1] < 0){  //if error
        perror("Unable to create child");
        //include in history
        exitStatus = -127;
        firstSequenceNumber++;
        add_history(cmd, exitStatus);
        return;
      }

      if (pids[pipenum-1] == 0) { //if child
        inchild = 1;

        if (pipenum-1 != pipecount) { //if not last command
          //set up write
          dup2(fd[(pipenum-1)*2+1], 1);
        }
        if (pipenum != 1) {  //if not first command
          //set up read
          dup2(fd[(pipenum-2)*2], 0);
        }
        //close all file descriptors
        for (size_t i = 0; i < pipecount*2; i++) {
          close(fd[i]);
        }
        break;
      }

      if (pipenum-1 != pipecount) { //if not last command
        memset(args, 0, MAX); //0 out args
        size=0; //reset size of args
        pipenum++;
        continue;
      }

      //close all file descriptors
      for (size_t i = 0; i < pipecount*2; i++) {
        close(fd[i]);
      }

      for (int i = 0; i < pipenum; i++) {
        waitpid(pids[i], &exitStatus, 0);

        if (WIFEXITED(exitStatus)) {
          exitStatus = WEXITSTATUS(exitStatus);
        }
        if (exitStatus != 0) {
          temp = 127;
        }
      }

      //include in history
      exitStatus = temp;
      firstSequenceNumber++;
      add_history(cmd, exitStatus);
      return;
    }
  }

  count = size;
  for (size_t i = 0; i < count; i++) {
    char *element = args[i];

    //handle >
    if (element[0] == '>') {
      if (element[1] == '\0' && i+1 < count) {    //if there is a space between > and file name
        file = args[i+1];
        num_elements = 2;
        shift_array(args, i, num_elements);
        count = count - num_elements;
        newfd = open(file, O_CREAT | O_WRONLY, 0664);
      }

      else if (strlen(args[i]) > 1){
        char file[(strlen(args[i]))];
        memcpy(file, &element[1], strlen(args[i]));
        file[strlen(args[i])-1] = '\0';
        num_elements = 1;
        shift_array(args, i, num_elements);
        count = count - num_elements;
        newfd = open(file, O_CREAT | O_WRONLY, 0664);
      }

      if (newfd == -1) {
        perror("unable to open file");
        exitStatus = 127;
        add_history(cmd, exitStatus);
        return;
      }
      else {
        stdout = dup(1);
        dup2(newfd, 1);
      }
    }
    //handle <
    if (element[0] == '<') {
      if (element[1] == '\0' && i+1 < count) {    //if there is a space between > and file name
        file = args[i+1];
        num_elements = 2;
        shift_array(args, i, num_elements);
        count = count - num_elements;
        newfd = open(file, O_RDONLY, 0664);
      }

      else if (strlen(args[i]) > 1){
        char file[(strlen(args[i])-1)];
        memcpy(file, &element[1], strlen(args[i]));
        file[strlen(args[i])-1] = '\0';
        num_elements = 1;
        shift_array(args, i, num_elements);
        count = count - num_elements;
        newfd = open(file, O_RDONLY, 0664);
      }


      if (newfd == -1) {
        perror("unable to open file");
        //include in history
        firstSequenceNumber++;
        exitStatus = 127;
        add_history(cmd, exitStatus);
        return;
      }
      else {  //change stdin fd to newfd
        stdin = dup(0);
        dup2(newfd, 0);
      }
    }
  }


  // if statements to identify the command and its arguments

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
        //include in history
        firstSequenceNumber++;
        exitStatus = 1;
        add_history(cmd, exitStatus);
      }
      else{               //print name of new directory location
        fprintf(stderr, "%s\n", args[1]);
        //include in history
        firstSequenceNumber++;
        exitStatus = 0;
        add_history(cmd, exitStatus);
      }
    }
    else if (strncmp(args[0], "history", MAX) == 0) {  //if command is history
      //include in history
      firstSequenceNumber++;
      exitStatus = 0;
      add_history(cmd, exitStatus);

      print_history(firstSequenceNumber);
    }
    else{     //if command is external or unkown
      //printf("about to exec\n");
      //infoFiles();
      int pid = executeExternalCommand(str, args);
      exitStatus = pid;
      //include in history
      firstSequenceNumber++;
      add_history(cmd, exitStatus);
    }
    if (stdin != -2) {  //if stdin was redirected then reset
      dup2(stdin, 0);
    }

    if (stdout != -2) { //if stdout was redirected then reset
      dup2(stdout, 1);
    }

    if (inchild) {
      clear_history();
      _exit(exitStatus);
    }

    return;
}



//executeExternalCommand handles external and unrecognized commands passed from smash

int executeExternalCommand(char *str, char **args){
  int status;
  int pid = fork();

  if(pid < 0){  //if error
    perror("Unable to execute external command");
    status = -1;
  }
  else if(pid > 0){ //if parent
    wait(&status);
  }
  else if(pid == 0){  //if child
    execvp(args[0], args);
    // if execvp fails exit with error 127
    char *val = args[0];
    perror(val);
    clear_history();
    _exit(127);
  }
  if (WIFEXITED(status)) {
    status = WEXITSTATUS(status);
  }
  return status;
}

 //shifts array to remove elements starting at pos and removing num_elements
void shift_array(char *array[], int pos, int num_elements ){
  num_elements --;
  int i = pos + num_elements;
  while (array[i] != NULL) {
    array[pos] = array[i+1];
    pos ++;
    i ++;
  }
  array[pos] = NULL; //NULL terminate the array
  return;
}
