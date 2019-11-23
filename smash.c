//NAME
//  smash.c
//
//SYNOPSIS
//  smash
//
//DESCRIPTION
//  driver program for a simplified shell
//
//LIMITATIONS
//  Lines of text are limited to a max of 4096 chars.
//
//AUTHOR
//  Adam Berridge

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "smash.h"
#include "history.h"

#define MAXLINE 4096
//---------------------------------------------------------------------
//main -- the main function
//---------------------------------------------------------------------
int main (int argc, char **argv) {
  setvbuf(stdout,NULL,_IONBF,0);  //Disable buffering in the stdout stream
  setvbuf(stderr,NULL,_IONBF,0);  //Disable buffering in the stderr stream

  char bfr[MAXLINE];
  init_history();  //Builds data structures for recording cmd history

  fputs("$ ", stderr);

  //loop while reading commands until EOF or error
  while (fgets(bfr, MAXLINE, stdin) != NULL){
    bfr[strlen(bfr) - 1] = '\0'; //replace newline with NULL

    executeCommand(bfr);
    fputs("$ ", stderr);
  }
  clear_history();
  return 0;
}
