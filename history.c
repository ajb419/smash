// NAME: history.c
//
// DESCRIPTION: history.c implements the defined "interface" history.h
//
// AUTHOR: Adam Berridge

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

struct Cmd *history[MAXHISTORY];
static int firstCheck = 1; //check if history[9] is null

void init_history(void){
  for (int i = 0; i < MAXHISTORY; i++) {
    history[i] = malloc(sizeof(struct Cmd));
    history[i]->cmd = NULL;
    history[i]->exitStatus = 0;
  }
}

void add_history(char *cmd, int exitStatus){
  int index = 0;

  while (index < (MAXHISTORY-1) && history[index]->cmd != NULL) {  //advance to next open position or last index
    index ++;
  }

  if (index < 9) {
    history[index]->cmd = strndup(cmd, strlen(cmd));
    history[index]->exitStatus = exitStatus;
  }

  else if(firstCheck == 1){
    history[index]->cmd = strndup(cmd, strlen(cmd));
    history[index]->exitStatus = exitStatus;
    firstCheck = 0;
  }

  else {    //shift everything in history left
    char *temp = history[0]->cmd;
    for (int i = 0; i < 9; i++) {
      history[i]->cmd = history[i+1]->cmd;
      history[i]->exitStatus = history[i+1]->exitStatus;
    }
    free(temp);
    history[index]->cmd = strndup(cmd, strlen(cmd));
    history[index]->exitStatus = exitStatus;
  }
/*
  printf("----------END OF ADD-------------\n");
  for (int i = 0; i < 10; i++) {
    printf("[%d] %s\n", history[i]->exitStatus, history[i]->cmd);
  }
  */
}

void clear_history(void){
  for (int i = 0; i < MAXHISTORY; i++) {
    if(history[i]->cmd != NULL){
      free(history[i]->cmd);
    }
    free(history[i]);
  }
}

void print_history(int firstSequenceNumber){
  int len = 0;
  while (len < (MAXHISTORY) && history[len]->cmd != NULL) {
    len++;
  }
  firstSequenceNumber = firstSequenceNumber-len +1;
  for (int i = 0; i < len; i++) {
    printf("%d [%d] %s\n",firstSequenceNumber+i, history[i]->exitStatus, history[i]->cmd);
  }

}
