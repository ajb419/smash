// DESCRIPTION
//  smash.h -- Defines function prototypes and symbols for smash.c

// Define function prototypes implemented in smash.c
#ifndef SMASH_H
#define SMASH_H

void executeCommand(char *str);  //Returns exit status
int executeExternalCommand(char *str, char **args);  //Returns exit status

#endif
