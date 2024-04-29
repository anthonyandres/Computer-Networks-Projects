/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>



#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */
//#define PATH "/home/condor/Desktop" /*SERVER FILES LOCATED ON VM DESKTOP*/
int echod(int);
void reaper(int);

int main(int argc, char **argv)
{
	int 	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;

	switch(argc){
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);

	while(1) {
	  client_len = sizeof(client);
	  new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
	  if(new_sd < 0){
	    fprintf(stderr, "Can't accept client \n");
	    exit(1);
	  }
	  switch (fork()){
	  case 0:		/* child */
		(void) close(sd);
		exit(echod(new_sd));
	  default:		/* parent */
		(void) close(new_sd);
		break;
	  case -1:
		fprintf(stderr, "fork: error\n");
	  }
	}
}

/*	echod program	*/
int echod(int sd)
{
	char txt[BUFLEN] = ".txt";
	char filename[BUFLEN];
	char data[99999];
	char buffer[99999];
	char	*bp, buf[BUFLEN];
	int 	n, m, bytes_to_read, wordCount = 0;
	FILE *file;
	
	
	n = read(sd, buf, BUFLEN);
	strcat(buf, txt); 
	strcpy(filename, buf);
	printf("received: %s\n", buf);
	printf("opening file: %s\n", filename);
	
	file = fopen(filename, "r");
	
	if(file != NULL){//check if file is valid
		printf("file exists\n");
		while(fgets(buffer, sizeof(buffer), file)){//read one line from file per loop
			strcat(data, buffer);//concatenate single read line onto data array
		}
		fclose(file);
	}
	else{
		char *error = "//////////////////////////////////////////////////// file does not exist /////////////////////////////////////////////////";
		printf("file does not exist, sending message to client\n");
		write(sd, error, sizeof(error));
	}
	
	printf(data);
	//TODO write data to client with maximum packet size of 100 bytes
	int sizecheck = (sizeof(data)) / 100;

	m = write(sd, data, 100);
	close(sd);

	return(0);


// 	FILE *fptr;
// 	char *bp, buf[BUFLEN], data[BUFLEN], error[BUFLEN];
// 	char *txt = ".txt";
// 	char *pathname = "/home/condor/Desktop";
// 	int n, bytes_to_read;

// 	printf("start\n");
	
// 	read(sd, buf, BUFLEN); /*read file name*/
// 	//char* filename = malloc(strlen(buf) + strlen(txt) + 1); //plus 1 for terminating character
// 	//char *filename = malloc(strlen(4) + 1);
// 	char *filename = buf;
// 	//filename = buf;
// 	//sprintf(filename, "%s%s", buf, txt);
// 	printf(*filename);

// 	if(access(filename, F_OK) == 0){ /*check if file exists*/
// 	//exists
// 		printf("\nfile exists\n");
// 		fptr = fopen("lab 3/" + *filename, "r");  /*read file and send file data*/
// 		printf("reading from file: " + *filename);
// 		char *filedata;
		
// 		while(fgets(data, BUFLEN, fptr) != NULL){ /*fgets reads a line from stream fptr, puts it into data*/
// 			//put data into array


// 			if(feof(fptr)){ /*check for end of file*/
				
// 			}
// 			else{ /*error interrupting read occured*/
				
// 			}
// 		}
// 		printf(data);
// 		fclose(fptr);
// 	} 
// 	else{
// 	//does not exist
// 	printf("\nfile does not exist\n");
// 	int e = sizeof("error: file does not exist");
// 	char errormsg[BUFLEN] = "error: file does not exist";
// 	write(sd, error, e);
		
// 	}
// 	close(sd);
// 	return(0);
// 	// char	*bp, buf[BUFLEN];
// 	// int 	n, bytes_to_read;

// 	// while(n = read(sd, buf, BUFLEN)) 
// 	// 	write(sd, buf, n);
// 	// close(sd);

// 	// return(0);
}

/*	reaper		*/
void	reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
