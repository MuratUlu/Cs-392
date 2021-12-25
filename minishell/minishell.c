//Murat Ulu
//I pledge my honor that I have abided by the Stevens Honor System.

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <sys/wait.h>

#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"


//================== Compares to see if minishell calls cd ================================
bool startsWithCD(char const *input){
	if(input[0] != 'c'){
		return false;
	}
	if(input[1] != 'd'){
		return false;
	}
	return true;
}

//================== returns number of words in given string ============
int numWords(char *str){
	int words = 1;
	for(int i = 0; str[i] != '\0'; i++){
		if(str[i] == ' ' || str[i] == '\t'){
			words++;
		}
	}
	return words;
}


//================== frees specific 256 set mallocs ============
void fullFree(char **mem){
	for(int i = 0; i < 256; i++){
		free(mem[i]);
	}
	free(mem);
}


//============================ Signal handling ======================================
volatile sig_atomic_t interrupted = 0;

void catch_signal(int sig) {	
	interrupted = 1;
}

//============================ Main ======================================
int main(int argc, char **argv){
	while(true){	
		interrupted = 0;
		
		struct sigaction sa;
				
		memset(&sa, 0, sizeof(struct sigaction));	
		sa.sa_handler = catch_signal;
		sigemptyset(&sa.sa_mask);
				
		if(sigaction(SIGINT, &sa, NULL) < 0){
			fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
			continue;
		}
		if(interrupted == 1){
			continue;
		}
		
		
		
		//prints cwd prompt
		char cwd[PATH_MAX];
		if(getcwd(cwd, sizeof(cwd)) == NULL){
			fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
		else{
			printf("[%s%s%s]$ ", BRIGHTBLUE, cwd, DEFAULT);
		}
		
		//cwd prompt wasn't printing, found this on stack overflow and it works now
		fflush(stdout);
		
		//--------------------------------------------
		
		char buffer[256];		
		//reading from stdin into buffer
		if(read(0, buffer, 256) == -1){
			if(interrupted == 1){
				printf("\n");
				continue;
			}
			else{
				fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
				return EXIT_FAILURE;
			}
		}		
		buffer[strcspn(buffer, "\n")] = 0;
		
		//------Code for cd--------------------------------------
		if(startsWithCD(buffer) == true){
			//sets home directory string/path into char pointer "homedir"
			const char *homedir;
			if((homedir = getpwuid(getuid())->pw_dir) == NULL){
				fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
				continue;
			}
			
		//parses given dir for cd
			//too many arguments given
			if(numWords(buffer) > 2){
				fprintf(stderr, "Error: Too many arguments to cd.\n");
				continue;
			}
			//when no arguments are supplied
			else if(numWords(buffer) == 1){
				chdir(homedir);
			}
			else{
				char *givenDir = buffer + 3;
				char newCWD[PATH_MAX];
				
				//for '~' input as cd argument, chdir's into home directory
				if(givenDir[0] == '~'){
					chdir(homedir);
				}
				//for going deeper into cwd
				else if(givenDir[0] != '/'){
					if(getcwd(newCWD, sizeof(newCWD)) == NULL){
						fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
						continue;
					}
					//adds "/path" to end of cwd working directory string so it can be used on chdir
					strcat(newCWD, "/");
					strcat(newCWD, givenDir);
					
					if(chdir(newCWD) == -1){
						fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n",givenDir, strerror(errno));
						continue;
					}
				}
				else{
					if(chdir(givenDir) == -1){
						fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n",givenDir, strerror(errno));
						continue;
					}
				}
			}
		}
		
		//-------------------------------------------- EXIT
		else if(strcmp(buffer, "exit") == 0){
			return EXIT_SUCCESS;
		}
		//------------------------------------------- exec
		else{
			size_t length = strlen(buffer);
			if(buffer[length - 1] == '\0'){
				buffer[length - 1] = '\n';
			}
			
			char **given = malloc(256*sizeof(char*));
			if(given == NULL){
				fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
				break;
			}
			for(int i = 0; i < 256; i++){
				given[i] = malloc(256*sizeof(char));
				if(given[i] == NULL){
					fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
					break;
				}
			}
			
			char * arg1 = strtok(buffer, " \n");
			int i = 0;
			while (arg1 != NULL){
				strcpy(given[i], arg1);
				arg1 = strtok(NULL, " \n");
				i++;
			}
			
			free(given[i]);
			given[i] = 0;
			
			pid_t pid;
			pid_t cpid;
			if((pid = fork()) < 0){
				fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
				fullFree(given);
				continue;
			}
			else if (pid > 0){
				cpid = wait(NULL);
				//-----------------------------
				if(interrupted == 1){
					fullFree(given);
					printf("\n");
					continue;
				}
				//-----------------------------
				if(cpid == -1){
					if(interrupted == 1){
						fullFree(given);
						printf("\n");
						continue;
					}
					else{
						fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
						fullFree(given);
						continue;
					}
				}
			}
			else{
				if(execvp(given[0], given) < 0){
					fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
					fullFree(given);
					return EXIT_FAILURE;
				}
			}
			fullFree(given);
		}
		
	}
		
	return 1;
}
