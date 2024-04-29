/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>


/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
struct pdu{
	char type;
	char data[100];
};

int main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	char    filebuf[100];
	char 	dataBuf[100];
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
    FILE *file;                                                                            
	int bytesize;
	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);
//while(1){
		struct pdu cPDU;
		recvfrom(s, &cPDU, sizeof(cPDU), 0,(struct sockaddr *)&fsin, &alen);
		//printf(buf);
		printf("%c\n", cPDU.type);
		printf("%s\n", cPDU.data);
		//printf("opening file: %s\n", buf);
		
		//buf[strlen(buf) - 1] = '\0';

		file = fopen(cPDU.data, "r");
		struct pdu erro;
	
		if(file != NULL){//check if file is valid
		erro.type = 'A';
			(void) sendto(s, &erro, sizeof(erro) + 1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
			printf("file %s exists\n", cPDU.data);
			fseek(file, 0, SEEK_END);
			bytesize = ftell(file);
			printf("file size: %d\n", bytesize);
			int conbytesize = htonl(bytesize);
			(void) sendto(s, &conbytesize, sizeof(conbytesize), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			int numPackets = (bytesize/100) + 1;
			printf("%d packets must be sent\n", numPackets);
			fseek(file, 0, SEEK_SET);//sets stream pointer back to start of file
			
			int i;
			struct pdu sentFile; 
			for(i = 0; i < numPackets; i++){//sending required num of packets
			printf("sending package %d\n", i + 1);
				if(i == numPackets - 1){ // if last package, denote 'F' type, else 'D' type
					sentFile.type = 'F';
				}
				else{
					sentFile.type = 'D';
				}
				fread(filebuf, 100, 1, file);//read 100 bits from file
				strcpy(sentFile.data, filebuf);
			
				(void) sendto(s, &sentFile, sizeof(sentFile) + 1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
			
			}
			printf("file finished sending\n");

			//send(s, data, sizeof(data), 0) != -1
			// for(i = 0; i < numPackets; i++){
			// 	fread(filebuf, 100, 1, file);
			// 	strcpy(data, filebuf);
			// 	write(s, data, sizeof(data));					
			// }

			//TODO: send structs of data packets of 100 then have client read those structs as either type D or F then create the file based on the struct.data

			fclose(file);
		}
		else{
			erro.type = 'E';
			char error[999] = "//////////////////////////////////////////////////// file does not exist /////////////////////////////////////////////////";
			strcpy(erro.data, error);
			printf("file does not exist, sending message to client\n");
			(void) sendto(s, &erro, sizeof(erro) + 1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
		}

	// while (1) {
	// 	// if (recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *)&fsin, &alen) < 0){
	// 	// 	fprintf(stderr, "recvfrom error\n");
	// 	// }
	// 	// else{
	// 	// 	printf(buf);
	// 	// }

	// 	// (void) time(&now);
    //     // 	pts = ctime(&now);

	// 	(void) sendto(s, pts, strlen(pts), 0,
	// 		(struct sockaddr *)&fsin, sizeof(fsin));
	// }
//}
}
