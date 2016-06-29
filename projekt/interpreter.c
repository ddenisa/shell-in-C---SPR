
#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>


void readconfig(int *timeout, char *file){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	char * token;
	fp = fopen(CONFIG, "r");
	if (fp == NULL)
	{
		
		*timeout = DEFAULTTIME;
		
	}
	else
	{
		while ((getline(&line, &len, fp)) != -1){
			token = strtok(line, TOKEN);
			if ((strcmp(token,TIME_PERIOD) == 0)){
				*timeout = atoi(strtok(NULL, TOKEN));
			}
			if ((strcmp(token,HISTORY)==0)){
				strcpy(file,trim_command(strtok(NULL, TOKEN)));				
			}
		}
	}
}


int waitPeriod(pid_t pid){
	if(pid) {
     int stat_val;
pid_t child_pid;
	int time = 0;
		
	while(time < TIME_PERIOD){
		child_pid = waitpid(pid, &stat_val, WNOHANG);     
		if (child_pid == pid){
			break;
			} else if (child_pid == 0) {
				time++;
				sleep(1);
			}
		}
	
		if (time == TIME_PERIOD){
			printf("Process ended. Execution time was over. \n");
			kill(pid, SIGINT);
			if (kill(pid, 0) == 0){
				kill(pid, SIGKILL);
			}
		}				
        
        if(WIFEXITED(stat_val)) {
			return WEXITSTATUS(stat_val);
		}
		
		return 0;
    } else {
		return -1;
	}
}

void splitCommand(char ** parameters, char * command, int count){
	char * cm = strtok(command, " ");
	int i = 0;
	while (cm != NULL){		
		parameters[i] = malloc(strlen(cm) * sizeof(char *));
		if(parameters[i] == NULL){
			openlog ("Interpreter", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
			syslog (LOG_CRIT, "Allocation error");
			closelog();
		}
       parameters[i] = cm;
        i++;
        cm = strtok (NULL, " ");
    }       
    i++;
    parameters[i] = NULL;

}

int getParamsCount(char command[]){
	int i;
	int count = 1;
	int com_length = strlen(command);
	for(i=0; i<com_length; i++){
		if (command[i] == ' '){
			count++;
		}
	}
	return count;
}

void getHistory(FILE * history_file, char ** history_commands){	
	char line[MAX_INPUT];
	int i = 0;
	int j;
	rewind(history_file);
	while (fgets(line, MAX_INPUT, history_file)!= NULL){
		history_commands[i] = malloc(strlen(line) * sizeof(char *));
		if(history_commands[i] == NULL){
			openlog ("Interpreter", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
			syslog (LOG_CRIT, "Allocation error");
			closelog();
		}		
		history_commands[i] = strdup(line);
		for(j=0; j<MAX_INPUT; j++){
			if (history_commands[i][j] == '\n'){
				history_commands[i][j] = '\0';
				break;
			}
		}
		i++;
	}
}

int getNumberOfRows(FILE * history_file){
	rewind(history_file);
	int lines = 0;
	char ch;
	while(!feof(history_file)){
		ch = fgetc(history_file);
		if(ch == '\n'){
			lines++;
		}
	}
	return lines;
}

void printCommands(char ** history_commands, int lines){
	int i = 0;
	int j = 0;
		
	
	for(j = i; j < lines; j++){
		printf("[%d] %s\n", j, history_commands[j]);
	}
}

int runCommand(pid_t pid, char * command, char ** parameters, int child){
	int count = 0;
	//const char *path = getenv("PATH");
	//const char *home = getenv("HOME");

	switch(pid) {
		case -1 :
			return -1;
		case 0 :
			count = getParamsCount(command);
			//parameters = malloc((count+1) * sizeof(char *));
			if(parameters == NULL){
				openlog ("Interpreter", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
			syslog (LOG_CRIT, "Allocation error");
			closelog();
			}
			splitCommand(parameters, command, count);
			if (strncmp("cd", parameters[0], 2) == 0){
					chdir(parameters[1]);
				
			} else {
				execvp(parameters[0], parameters);
				//return 0;
			}
			//free(parameters);
			return 0;
		default :
			child = waitPeriod(pid);
			printf("Child status: %d\n", child);
			command[0] = 0;
			free(command);			
			return 0;
	}
}

char *trim_command(char * string){
	int l = strlen(string);
	int i;
	for (i = 0; i<l; i++){
		if (string[i] == '\n'){
			string[i] = '\0';
		}
	}
	return string;
}



int main(){
	
	
	char * command;
	
	FILE * history;
	
	char cwd[MAX_INPUT];
	
	pid_t pid = 0;	
	int child = 0;	
	
	int number_of_rows = 0;
	int count = 0;
	
	char ** history_commands;	
	char* parameters[30];
	
	openlog ("INTERPRETER", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_INFO, "Program started by User %d", getuid ());
	closelog();
	
	
	printf("                              Safe comand line interpreter                         \n");
	printf("                    Type \"quit\" to cancel interpreter or command                  \n");
	printf("                    Type \"history\" to show full command history                  \n");
	printf("                    Type \"history number\" to execute command from history       \n");
	printf("________________________________________________________________________________________\n");
	sleep(1);
	
	history = fopen(HISTORY, "a+");
	if (history == NULL){
		printf("Could not open history file\n");
		return -1;
	}
	
	while(1){
		getcwd(cwd, sizeof(cwd));
		printf("Command: [%d]%s : ", child, cwd);
				
		command = malloc(MAX_INPUT * sizeof(char *));
		if(command == NULL){
			openlog ("INTERPRETER", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
			syslog (LOG_CRIT, "Allocation error");
			closelog();
		}
		fgets(command, MAX_INPUT, stdin);
		trim_command(command);
				
		if (strncmp(QUIT, command, 4) == 0) {
			fclose(history);
			return EXIT_SUCCESS;
			//exit(0);
		} else if (strncmp("history", command, 7) == 0) {														
			number_of_rows = getNumberOfRows(history);												
			history_commands = malloc(number_of_rows * sizeof(char *));
			if(history_commands == NULL){
				openlog ("Interpreter", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
			syslog (LOG_CRIT, "Allocation error");
			closelog();
			}						
			getHistory(history, history_commands);
			count = getParamsCount(command);
			
				if (strcmp("history", command) == 0){
					printf("HISTORY OF COMMANDS\n");
					printCommands(history_commands, number_of_rows);
				} else {
					char * cm = strtok(command, " ");
					cm = strtok(NULL, " ");
				
					int x;
					x = atoi(cm);
					
					if (x >= number_of_rows){
						printf("Command does not exist\n");
						continue;
					}
					
					command = strdup(history_commands[x]);
					count = getParamsCount(command);
					trim_command(command);
					
					fprintf(history, "%s\n", command);

					pid = fork();
					runCommand(pid, command, parameters, child);
				}
			
													
		} else if (command[0] != 0){
			fprintf(history, "%s\n", command);
			
			pid = fork();
			runCommand(pid, command, parameters, child);			
		}
	}
	
	fclose(history);
	return 0;
}
