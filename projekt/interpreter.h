#ifndef __BASIC__
#define __BASIC_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define QUIT "quit"
#define MAX_INPUT 1024
#define MAX_PARAM 15

#define CONFIG "config.cfg"

#define	TOKEN	"="
#define DEFAULTTIME	20
#define HISTORY	"history"
#define TIME_PERIOD	"time_period"


void readconfig(int *timeout, char *file);
int waitPeriod(pid_t pid);
void splitCommand(char ** parameters, char * command, int count);
int getParamsCount(char command[]);
void getHistory(FILE * history_file, char ** history_commands);
int getNumberOfRows(FILE * history_file);
void printCommands(char ** history_commands, int lines);
void prepareStatements(char ** parameters, char * command, int count);
int runCommand(pid_t pid, char * command, char ** parameters, int child_status);
char *trim_command(char * string);


#endif /* __BASIC__ */
