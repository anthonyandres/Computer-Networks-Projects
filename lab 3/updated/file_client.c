/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>



#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];

	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}

	// printf("Name a file: \n");
	// while(n=read(0, sbuf, BUFLEN)){	/* get user message */
	// write(sd, sbuf, n-1);//-1 to remove \n		/* send it out */
	// printf("sent out:%s", sbuf);
	// }
	printf("Transmit: \n");
	bool error = false;
	while(n=read(0, sbuf, BUFLEN)){	/* get user message */
	   write(sd, sbuf, n-1);		/* send it out */
	   printf("Receive: \n");
	   bp = rbuf;
	}
	   //since server can only send up to 100 bytes at a time,
	   //once a packet size of less than 100 is sent, 
	   //that means that is the last packet to be sent
	   //read loop will break
	   while ((i = read(sd, bp, 100)) == 100 || error == false){//note comparator can be == or >= but not <=, the break condition should be bytes read is less than 100
	 	if(bp[0] == '/'){
			printf("file not found\n");
			error = true;
		}
		bp += i;
	 	bytes_to_read -=i;
	   }
	
	   
	   //data should now be in rbuf, use this array to write to a file
	   if(error == false){
	   FILE *downloadFile = fopen(rbuf, "w");
	   int i;
	   if(downloadFile == NULL){
			printf("error creating downloaded file");
	   }
	   int strLen = strlen(rbuf);
	   for(i = 0; i < strLen; i++){
		   fprintf(downloadFile, "%c", rbuf[i]); //writing to file
	   }
	   
	//   write(1, rbuf, n); //<<this mig
	//   printf("Transmit: \n");
	}

	close(sd);
	return(0);
}
