//Murat Ulu
//I pledge my honor that I have abided by the Stevens Honor System.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin(){
	int valid = get_string(outbuf, MAX_MSG_LEN);
	if(valid == TOO_LONG){
		fprintf(stderr, "Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
		//doc doesnt say to exit
	}
	else if (send(client_socket, outbuf, strlen(outbuf), 0) < 0){
		fprintf(stderr, "Error: Failed to send message to server. %s.\n", strerror(errno));
		//can't use goto EXIT, out of scope
		return EXIT_FAILURE;
	}
	
	if(strcmp(outbuf, "bye") == 0){
		close(client_socket);
		printf("Goodbye.\n");
		return EXIT_FAILURE;
	}
	else{
		return EXIT_SUCCESS;
	}
}

int handle_client_socket(){
	int bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0);
	if(bytes_recvd == -1){
		fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n", strerror(errno));
	}
	if(bytes_recvd == 0){
		fprintf(stderr, "\nConnection to server has been lost.\n");
		close(client_socket);
		return EXIT_FAILURE;
	}
	
	inbuf[bytes_recvd] = '\0'; //Null terminate the string
	
	if(strcmp(inbuf, "bye") == 0){
		close(client_socket);
		printf("\nServer initiated shutdown.\n");
		return EXIT_FAILURE;
	}
	else{
		printf("\n%s\n", inbuf);
		return EXIT_SUCCESS;
	}
	
}

int main(int argc, char *argv[]){
	//Checks for improper usage syntax
	if(argc != 3){
		fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	int ip_conversion;
	int retval = EXIT_SUCCESS;
	
	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);	
	memset(&server_addr, 0, addrlen);
	
	//IP address conversion
	ip_conversion = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
	if(ip_conversion == 0){
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
		retval = EXIT_FAILURE;
		goto EXIT;
	} else if(ip_conversion < 0){
		fprintf(stderr, "Error: Failed to convert IP address. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	//Port conversion
	int port;
	if(parse_int(argv[2], &port, "port number") == false){
		retval = EXIT_FAILURE;
		goto EXIT;
	} else if(port < 1024 || port > 65535){
		fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
		retval = EXIT_FAILURE;
		goto EXIT;
	}	//port input is confirmed valid	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	
	
	//Prompting for a Username section
	bool usernameValid = false;
	while(usernameValid == false){
		printf("Enter a username: ");
		fflush(stdout);
		int valid = get_string(username, MAX_NAME_LEN + 1); //get_string() from util
		if(valid == NO_INPUT){
			continue;
		}
		else if(valid == TOO_LONG){
			fprintf(stderr, "Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
			continue;
		}
		else{
			usernameValid = true;
		}
	}
	printf("Hello, %s. Let's try to connect to the server.\n", username);
	
	
	//Establishing Connection section
	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	if(connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0){
		fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	int bytes_recvd;
	if((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0){
		fprintf(stderr, "Error: Failed to receive message from server. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;		
	}
	else if(bytes_recvd == 0){
		fprintf(stderr, "All connections are busy. Try again later.\n");
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	printf("\n%s\n\n\n", inbuf);
	
	if(send(client_socket, username, strlen(username), 0) < 0){
		fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	//Handing Activitity on File Descriptors section
	while(1){
		printf("[%s]: ",username);
		fflush(stdout);
		
		fd_set socket;
		FD_SET(STDIN_FILENO, &socket);
		FD_SET(client_socket, &socket);
		
		if(select(client_socket + 1, &socket, NULL, NULL, NULL) < 0 ){
			fprintf(stderr, "Error: select() failed. %s.\n", strerror(errno));
			retval = EXIT_FAILURE;
			goto EXIT;
		}
		if(FD_ISSET(STDIN_FILENO, &socket)){
			if(handle_stdin() == EXIT_FAILURE){
				break;
			}
		}
		else if (FD_ISSET(client_socket, &socket)){
			if(handle_client_socket() == EXIT_FAILURE){
				break;
			}
		}
	}
	
	if (fcntl(client_socket, F_GETFD) >= 0) {
		close(client_socket);
	}
	return retval;
	
	
	
	//If socket is open, close before terminating
	EXIT:
	    if (fcntl(client_socket, F_GETFD) >= 0) {
	        close(client_socket);
	    }
	    return retval;
}