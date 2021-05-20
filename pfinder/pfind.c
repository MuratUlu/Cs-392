//Murat Ulu
//"I pledge my honor that I have abided by the Stevens Honor System."

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>


int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};
int permission_valid;


//Dislpays the usage
void display_usage(char *progname) {
	printf("Usage: %s -d <directory> -p <permissions string> [-h]\n", progname);
}


//================= STUFF FROM PERMSTAT LAB HELPED =====================
/*
* Returns a string (pointer to char array) containing the permissions of the
* file referenced in statbuf.
* Allocates enough space on the heap to hold the 9 printable characters.
* The 9 characters represent the permissions of user (owner), group,
* and others: r, w, x, or -.
*/
char* permission_string(struct stat *statbuf) {
	char* per;
	per = (char *)malloc(sizeof(char) * 10);
	char* p = per;
	for (int i = 0; i < 9; i += 3){
		permission_valid = statbuf->st_mode & perms[i];
		if (permission_valid){
			*per = 'r';
		} else {
			*per = '-';
		}
		per++;
		permission_valid = statbuf->st_mode & perms[i+1];
		if (permission_valid){
			*per = 'w';
		} else {
			*per = '-';
		}
		per++;
		permission_valid = statbuf->st_mode & perms[i+2];
		if (permission_valid){
			*per = 'x';
		} else {
			*per = '-';
		}
		per++;
	}
	*per = '\0';
	return p;
}

//================= CHECKS IF PERM STRING IS EQUAL TO ONE GIVEN ==============

//Returns 1 if stat perm string of file is NOT same as one provided, 0 if they are same
int perm_equal(char* file_name, char* provided_permis){
	
	int is_equal = 1;
	
	struct stat statbuf;
	if (lstat(file_name, &statbuf) < 0) {
		fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", file_name, strerror(errno));
	    return EXIT_FAILURE;
	}
	
	char *perms = permission_string(&statbuf);
	//printf("file permission: %s\n", perms);
	//printf("provided permission: %s\n", provided_permis);
	if(strcmp(perms, provided_permis) == 0){
		is_equal = 0;
	}
	free(perms);
	return is_equal;
}


//================= CHECKS If THE PERMISSIONS STRING PROVIDED IS VALID ===================


//Returns 1 if permissions string is invalid, 0 if it is valid
int is_perm_valid(char* perm_string){
	if(strlen(perm_string) != 9){
		return 1;
	}
	
	for(int i = 0; i < strlen(perm_string); i++){
		if( (i % 3) == 0 ){
			if((perm_string[i] != 'r') && (perm_string[i] != '-')){
				return 1;
			}
		}
		else if ( (i % 3) == 1){
			if((perm_string[i] != 'w') && (perm_string[i] != '-')){
				return 1;
			}
		}
		else{
			if((perm_string[i] != 'x') && (perm_string[i] != '-')){
				return 1;
			}
		}
	}
	return 0;
}


//=========================== STAT FULLPATH BUT COOLER ===========================================


//Recursively goes through given directory and prints file names with specified positions
int file_displayer(char* dir_string, char* permis_string){
	char path[PATH_MAX];
	if (realpath(dir_string, path) == NULL){
		fprintf(stderr, "Error: Cannot get full path of file '%s'. %s.\n", dir_string, strerror(errno));
		return EXIT_FAILURE;
	}

	DIR *dir;
	if((dir = opendir(path)) == NULL){
		fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", path, strerror(errno));
		return EXIT_FAILURE;
	}
	
	struct dirent *entry;
	struct stat sb;
	char full_filename[PATH_MAX+1];
	size_t pathlen = 0;
	
	full_filename[0] = '\0';
	if (strcmp(path, "/")){
		strncpy(full_filename, path, PATH_MAX);
	}
	//Add + 1 for the trailing '/' that we add
	pathlen = strlen(full_filename) + 1;
	full_filename[pathlen - 1] = '/';
	full_filename[pathlen] = '\0';
	
	while ((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0){
			continue;
		}
		strncpy(full_filename + pathlen, entry->d_name, PATH_MAX - pathlen);
		if(lstat(full_filename, &sb) < 0){
			fprintf(stderr, "Error: Cannot stat file '%s'. %s.\n", full_filename, strerror(errno));
			continue;
		}
		if (entry->d_type == DT_DIR){
			//changed to recursively enter each directory when possible
			if(perm_equal(full_filename, permis_string) == 0){
				printf("%s\n", full_filename);
			}
			file_displayer(full_filename, permis_string);
		}
		else{
			if(perm_equal(full_filename, permis_string) == 0){
				printf("%s\n", full_filename);
			}
		}
	}
	closedir(dir);
	return EXIT_SUCCESS;
	
}


//========================= MAIN FUNCTION =============================


int main (int argc, char *argv[]){
	
	int opt;
	
	int d_flag = 0;
	int p_flag = 0;
	char* given_dir;
	char* given_permis;
	
	//Case when no arguments are supplied
	if (argc == 1) {
		display_usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	//getopts
	while((opt = getopt(argc, argv, ":d:p:h")) != -1){
		switch(opt){
		
		case 'd':
			d_flag = 1;
			given_dir = optarg;
			break;
			
		case 'p':
			p_flag = 1;
			given_permis = optarg;
			break;
			
		//Help case 
		case 'h':
			display_usage(argv[0]);
			return EXIT_SUCCESS;

		//Case for missing args
		case ':':
			if(optopt == 'd'){
				printf("Error: Required argument -d <directory> not found.\n");
				return EXIT_FAILURE;
			}
			else{
				if(d_flag == 1){
					printf("Error: Required argument -p <permissions string> not found.\n");
					return EXIT_FAILURE;
				}
				else{
					printf("Error: Required argument -d <directory> not found.\n");
					return EXIT_FAILURE;
				}
			}
			
		//Unknown option case
		case '?':
			fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
			return EXIT_FAILURE;
			
		default: 
			break;
		}
		
	}
	
	//Case when -d is missing
	if((d_flag == 0) && (p_flag == 1)){
		printf("Error: Required argument -d <directory> not found.\n");
		return EXIT_FAILURE;
	}
	//Case when -p is missing
	if((d_flag == 1) && (p_flag == 0)){
		printf("Error: Required argument -p <permissions string> not found.\n");
		return EXIT_FAILURE;
	}

	//stat given_dir to make sure it is valid
	struct stat statbuf;
	//Checks if an invalid directory is passed to -d
	if (stat(given_dir, &statbuf) < 0) {
		fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", given_dir, strerror(errno));
	    return EXIT_FAILURE;
	}
	//Checks if argument is even a directory
	if (!S_ISDIR(statbuf.st_mode)){
		fprintf(stderr, "Error: '%s' is not a directory.\n", given_dir);
	    return EXIT_FAILURE;
	}

	//Checks if given permissions are valid
	if (is_perm_valid(given_permis) == 1){
		fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", given_permis);
		return EXIT_FAILURE;
	}
	
	//================ USING STAT FULLPATH BUT COOLER =================
	
	file_displayer(given_dir, given_permis);
	
	return EXIT_SUCCESS;
	
}