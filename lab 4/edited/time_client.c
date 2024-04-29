/* time_client.c - main */

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>                                                                                
#include <netdb.h>

#define	BUFSIZE 64

#define	MSG		"any message \n"


/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
struct pdu{
	char type;
	char data[100];
};

int main(int argc, char **argv)
{
	char	*host = "localhost";
	int	port = 3000;
	char	now[100];		/* 32-bit integer to hold time	*/ 
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, n, type;	/* socket descriptor and socket type	*/
	char buf[100];
	switch (argc) {
	case 1:
		break;
	case 2:
		host = argv[1];
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: UDPtime [host [port]]\n");
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;                                                                
        sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
        if ( phe = gethostbyname(host) ){
                memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect to %s %s \n", host, "Time");

	char choice[999];
//	bool con = true;
	char filename[999];
//	while(con == true){
	//while(1){
	struct pdu cPDU;
	cPDU.type = 'C';
	
		printf("What do you want to do?\n(a) download file\n(b) quit\n");
		scanf("%s", choice);
		if(strcmp(choice, "a") == 0){
			printf("enter file name and type, ex: foobar.txt\n");
			n = read(0, cPDU.data, 100);
			cPDU.data[n-1] = '\0';
		}
		else if(strcmp(choice, "b") == 0)
		{
			printf("goodbye\n");
			exit(0);
		}
		else{
			printf("invalid input\n");
			exit(0);
		}
//	}

	printf("sending PDU\n");
	write(s, &cPDU, n+1);
	int confileSize = 0;

	struct pdu err;
	recv(s,&err, sizeof(err), 0);
	if(err.type == 'E'){
		printf("%s", err.data);
		exit(0);
	}


	recv(s, &confileSize, sizeof(confileSize), 0);
	int fileSize = ntohl(confileSize);
	printf("size of file to be downloaded: %d\n", fileSize);

	int check;
	struct pdu sentFile;
	bool loop = true;
	char content[999];
	while(loop){
		printf("downloading %s...\n", cPDU.data);
		recv(s, &sentFile, sizeof(sentFile), 0);
		strcat(content, sentFile.data);
		if(sentFile.type == 'F'){
			loop = false;
		}
	}

   FILE *downloadFile = fopen(cPDU.data, "w");
   int i;
   if(downloadFile == NULL){
		printf("error creating downloaded file");
   }

   for(i = 0; i < fileSize; i++){
	   fprintf(downloadFile, "%c", content[i]); //writing to file
   }
	printf("file created\n");
	//recv(s, buf, 100, 0); //idk if i have to use a flag for recv
	//printf(buf);
	//write(s, filename, strlen(filename));

	// /* Read the time */

	// n = read(s, (char *)&now, sizeof(now));
	// if (n < 0)
	// 	fprintf(stderr, "Read failed\n");
	// write(1, now, n);

// struct pdu spdu;
// spdu.type = 'C';
// n = read(0, spdu.data, 100);

//client side, 4 types of message, client only sends part C, server can send D F or E in the same way 0 is error and 1 is valid
//D is for if data is still being transmitted
//F is for when this is the last packet sent of the file data
//we need to figure out how many packets need to be sent
//dump data into buffer, then figure out number of packets
//client knows what to do given a D or an F
//point to struct and can access data that way
//use a switch statement when receiving different data types
	//}

	exit(0);
	
}
