/*
functionalities of a Peer:
- register its content to index server
- find address of a content server peer from index server
- index server <-> peer connection is UDP
- content download peer <-> peer is TCP
*/

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
#include <errno.h>


struct pdu{
	char type;
	char data[100];
    char Cdata[9999]; //subject to change, might dynamically allocate
    //https://www.programiz.com/c-programming/examples/structure-dynamic-memory-allocation
    int numPeers;
    char peerName[5][10];
    char fileName[5][10];
    int portNum;

};

//char* generateData(char peerName[10], char contentName[10], char ipAddr[15], char portNum[10]);

void extractData(char* data);

int main(int argc, char **argv)
{
/*
------------------------------------------------------------------------
                                    UDP CLIENT
------------------------------------------------------------------------
*/
	char	*host = "localhost";
	int	port = 3000;
	
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	UDP, n, type;	/* socket descriptor and socket type	*/
	char buf[100], choice[100];
    char peerName[10], contentName[10], portNum[10], ipAddr[15], address[25], dlPeer[10], dlContent[10];
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
        UDP = socket(AF_INET, SOCK_DGRAM, 0);
        if (UDP < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(UDP, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect UDP");

        printf("UDP client setup!\n");

/*
------------------------------------------------------------------------
                                 TCP SERVER
------------------------------------------------------------------------
*/    


    int 	TCP, sd, new_TCP, client_len, alen;
	struct	sockaddr_in server, client;

	/* Create a stream socket: TCP*/	
	if ((TCP = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket*/
	//bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(0);
        //^port number is uniquely generated with "htons(0)"
        //find the port number stored in server.sin_port
        //register port number to index server
        //IP address is in server.sin_addr.s_addr (will be 0.0.0.0 meaning "this machine")
	
	if (bind(TCP, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}
    if((listen(TCP, 5)) < 0){
        printf("listen error");
        exit(1);
    }

    alen = sizeof(struct sockaddr_in);
    getsockname(TCP, (struct sockaddr *)&server, &alen);
    //^stores the socket name within the server sockaddr struct

    printf("TCP server setup! waiting to connect\n");

/*
------------------------------------------------------------------------
                      REGISTERING W/ INDEX SERVER   
------------------------------------------------------------------------
*/
bool registered = false;
bool whileLoop = true;
char uniqueName[10];


    fd_set rfds, afds; // bit array, ready and active
    //int sock = socket(AF_INET, SOCK_STREAM, 0);
    FD_ZERO(&afds);//fd_zero initializes
    FD_ZERO(&rfds);
    FD_SET(TCP, &afds); // listening on a TCP socket [set the bit of whatever value TCP is to 1]
    FD_SET(0, &afds); // listening on stdin (user input) [set 0th bit of afds to 1]
    int userPrompt = 0;

    while(1){
    memcpy(&rfds, &afds, sizeof(rfds));//copy sockets each loop bc select is destructive
    
    if(userPrompt == 0){
        printf("------------------------------------\nWhat do you want to do?\n(a) download file\n(b) register content\n(c) de-register content\n(d) list registered content\n(e) exit\n------------------------------------\n");
    }

    if(select(FD_SETSIZE, &rfds, NULL, NULL, NULL)<0){
        perror("select error");
        exit(EXIT_FAILURE);
    };

    userPrompt = 1;
    if(FD_ISSET(0, &rfds)){//if user input...
        userPrompt = 0;

        scanf("%s", choice, 100);

            if(strcmp(choice, "a") == 0){
                struct pdu Spdu, Rpdu;
                char peerPort[20], peerIpAddr[20], contentCopy[10], peerCopy[10];
                int peerPortInt, peerIpAddrInt;
                printf("What is the name of the peer and their registered content?\nExample:\n   Peer1\n   test.txt\n");
                //NOTE: peer name and content are maximum 10 bytes
                scanf("%s", dlPeer);
                scanf("%s", dlContent);
                strcpy(contentCopy, dlContent);//copy file name onto contentCopy
                strcpy(peerCopy, dlPeer);//copy peer name onto peerCopy
                int i, n;
                int word1 = strlen(dlPeer);
                int word2 = strlen(dlContent);

                for(i = 0; i < (sizeof(dlPeer) - word1); i++){
                    strcat(dlPeer, "~");
                }
                for(i = 0; i < (sizeof(dlContent) - word2); i++){
                    strcat(dlContent, "~");
                }

                strcpy(Spdu.data, dlPeer);
                strcat(Spdu.data, dlContent);

                n = strlen(Spdu.data);
                Spdu.type = 'S';
                write(UDP, &Spdu, n+1);

                recv(UDP, &Rpdu, sizeof(Rpdu), 0);
                if(Rpdu.type == 'E'){
                    printf("------------------------------------\n!Error: no such content exists------------------------------------\n\n");
                    exit(0);
                }
                else if(Rpdu.type == 'S'){
                    printf("S type PDU received: proceeding with download...\n");
                    char buffer[30];
                    memset(buffer,0,sizeof(buffer));
                    strncpy(buffer, Rpdu.data + 0, 20);
                    extractData(buffer);   
                    strcpy(peerPort, buffer);

                    peerPortInt = atoi(peerPort);//peer port and ip address turned to ints
                    //peerIpAddrInt = atoi(peerIpAddr); 

                    memset(buffer,0,sizeof(buffer));

                    strncpy(buffer, Rpdu.data + 20, 20);
                    extractData(buffer);   
                    strcpy(peerIpAddr, buffer);  
                    memset(buffer,0,sizeof(buffer));

                    //at this point the correct port and IP address for the requested peer
                    //should be available in peerPort and peerIpAddr
                    printf("connecting to %s on\nport: %s\nip address: %s\n",peerCopy, peerPort, peerIpAddr);
                }
                /*TODO:
                    figure our how to connect to specific peer using their port and IP address
                    https://people.cs.rutgers.edu/~pxk/rutgers/notes/sockets/
                    use this to learn: step 3a
                */

                struct hostent *hp; //host information
                struct sockaddr_in serverAddress; //server address
                int TCPclient;
                //memset((char*)&serverAddress, 0, sizeof(serverAddress));
                //bzero(&serverAddress, sizeof(serverAddress));
                serverAddress.sin_family = AF_INET;
                serverAddress.sin_addr.s_addr = inet_addr(peerIpAddr);//specify ip address to connect to
                serverAddress.sin_port = htons(peerPortInt);//specify port to connect to
                
                TCPclient = socket(AF_INET, SOCK_STREAM, 0);
                // struct addrinfo hints, *res;
                // memset(&hints, 0, sizeof(hints));
                // hints.ai_family = AF_UNSPEC;
                // hints.ai_socktype = SOCK_STREAM;
                // getaddrinfo(peerIpAddr, peerPort, &hints, &res);
                // TCPclient = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                // if(connect(TCPclient, res->ai_addr, res->ai_addrlen) == -1){
                //     fprintf(stderr, "error in connect(): %s\n", strerror(errno));
                //     close(TCPclient);
                //     exit(1);
                // }
                // else{
                //     printf("Success!!!\n");
                //     exit(1);
                // }

                //doing this remotely will not have a proper connection occur since the ip address does not actually exist. there is no device
                if (connect(TCPclient, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1){
                    fprintf(stderr, "error in connect(): %s\n", strerror(errno));
                    close(TCPclient);
                    exit(1);
                }
                else{
                    printf("connected to server!\nWaiting for Download\n");
                    struct pdu Rpdu, Spdu;
                    strcpy(Spdu.data, contentCopy);
                    write(TCPclient, &Spdu, sizeof(Spdu));//write 256 bytes from Spdu on TCP connection
                    recv(TCPclient,&Rpdu, sizeof(Rpdu), 0);
                    printf("%s", Rpdu.Cdata); 
                }
            }
            
            else if(strcmp(choice, "b") == 0)
            {
                if(registered){
                    printf("------------------------------------\nthis peer is already registered.\n------------------------------------\n");
                    continue;
                }
                //UDP stuff
                struct pdu Spdu, Rpdu;
                Spdu.type = 'R';
                strcpy(Spdu.data, ""); //clear Spdu.data in the case of registering after de-registering
                strcpy(address, "");
                int portNumber = server.sin_port;
                (void) sprintf(portNum, "%d", portNumber);

                printf("------------------------------------\nenter the Peer name, content name, and IP address\nExample:\n   Bob\n   dylan.txt\n   255.10.1.55\n------------------------------------\n");
                scanf("%s", peerName);
                scanf("%s", contentName);
                scanf("%s", ipAddr);

                strcpy(uniqueName, peerName);

                int i;
                int word1 = strlen(peerName);
                int word2 = strlen(contentName);
                int word3 = strlen(portNum);

                for(i = 0; i < (sizeof(peerName) - (word1)); i++){
                    strcat(peerName, "~");
                }

                for(i = 0; i < (sizeof(contentName) - (word2)); i++){
                    strcat(contentName, "~");
                }

                for(i = 0; i < (sizeof(portNum) - (word3)); i++){
                    strcat(portNum, "~");
                }

                strcpy(address, portNum);
                strcat(address, ipAddr);

                strcpy(Spdu.data, peerName);
                strcat(Spdu.data, contentName);
                strcat(Spdu.data, address);

                int n = strlen(Spdu.data);

                write(UDP, &Spdu, n+1);
    /*
    ^what this whole process does is seperate the data in Rpdu.data with a bunch of ~'s
    for example, for the input:

    bob
    dylan.txt
    255.255.255.255

    we get the following in Rpdu.data:

    bob~~~~~~~dylan.txt~2994~~~~~~255.255.255.255

    the first 10 chars are for peer name "bob~~~~~~~"
    the next 10 chars are for content name "dylan.txt~"
    the next 10 chars are for port number "2994~~~~~~"
    the last remaining chars (a maximum of 15) are for ip address "255.255.255.255"
    */
                //printf("\n"); printf(Spdu.data); printf("\n"); //test printf

                bool loop = true;
                while(loop){
                    recv(UDP, &Rpdu, sizeof(Rpdu), 0);
                    if(Rpdu.type == 'E'){
                        printf("------------------------------------\nerror in peer name, please select a new peer name:\n------------------------------------\n");
                        strcpy(Spdu.data, ""); //clear Spdu.data
                        strcpy(peerName, ""); // clear peerName
                        //strcpy(uniqueName, ""); // clear uniqueName
                        scanf("%s",peerName);
                
                        word1 = strlen(peerName);
                        for(i = 0; i < (sizeof(peerName) - (word1)); i++){
                            strcat(peerName, "~");
                        }
                        strcat(Spdu.data, peerName);
                        strcat(Spdu.data, contentName);
                        strcat(Spdu.data, address);

                        printf("%s\n", Spdu.data);

                        int n = strlen(Spdu.data);
                        write(UDP, &Spdu, n+1); 
                    }
                    else if(Rpdu.type == 'A'){
                        loop = false;
                        printf("---------uniqueName: %s ---------------------------\ncontent registered successfully!\n------------------------------------\n\n", uniqueName);
                        registered = true;
            
                    }
                }
            }

            else if(strcmp(choice, "c") == 0){
            //content de-register
                if(strcmp(uniqueName, "") == 0){
                    printf("------------------------------------\ncannot de-register: peer is not registered\n------------------------------------\n\n");
                    
                }
                else{
                    struct pdu Spdu, Rpdu;
                    Spdu.type = 'T';
                    //printf("UNIQUEname: %s\n", uniqueName);//remove
                    strcpy(Spdu.data, uniqueName);
                    //printf("Spdu.data: %s\n", Spdu.data);//remove
                    write(UDP, &Spdu, 10);
                    recv(UDP, &Rpdu, sizeof(Rpdu), 0);
                    if(Rpdu.type == 'A'){
                        registered = false;
                    }
                    printf("------------------------------------\nPDU type %c:\nContent De-registered Successfully!\n------------------------------------\n\n", Rpdu.type);
                }
            }

            else if(strcmp(choice, "d") == 0){
            //list registered content
                struct pdu Spdu;
                struct pdu Rpdu;
                int i;
                Spdu.type = 'O';
                write(UDP, &Spdu, 10);//arbitrary size since only Spdu.type == 'O' matters

                recv(UDP, &Rpdu, sizeof(Rpdu), 0);
                
                for(i = 0; i < Rpdu.numPeers; i++){
                    printf("------------------------------------\nPeer %d | Peer name: %s | File name: %s\n", i, Rpdu.peerName[i], Rpdu.fileName[i]);
                }
                printf("------------------------------------\n\n");
            }

            else if(strcmp(choice, "e") == 0){
                printf("------------------------------------\ngoodbye\n------------------------------------\n\n");
                    //same process, deregister content when exiting
                    struct pdu Spdu, Rpdu;
                    Spdu.type = 'T';
                    //printf("UNIQUEname: %s\n", uniqueName);//remove
                    strcpy(Spdu.data, uniqueName);
                    //printf("Spdu.data: %s\n", Spdu.data);//remove
                    write(UDP, &Spdu, 10);
                    recv(UDP, &Rpdu, sizeof(Rpdu), 0);
                    if(Rpdu.type == 'A'){
                        registered = false;
                    }
                exit(0);
            }
            else{
                printf("------------------------------------\ninvalid input\n------------------------------------\n\n");
                exit(0);
            }
    }

    if(FD_ISSET(TCP, &rfds)){//if other peer wants to connect to me...
        printf("\nINCOMING CONNECTION!!\n");
            client_len = sizeof(client);
            new_TCP = accept(TCP, (struct sockaddr *)&client, &client_len);
            FD_SET(new_TCP, &afds);
            if(new_TCP < 0){
                fprintf(stderr, "false positive Can't accept client// \n");
                exit(1);
            }
            else{
                printf("TCP connection established!\n");
                struct pdu Rpdu, Spdu;
                int bytesize;
                char filebuf[300];

                recv(new_TCP, &Rpdu, sizeof(Rpdu), 0);//receive file name

                FILE *file = fopen(Rpdu.data, "r");//open file to read
                if(file != NULL){//if file exists
                    bytesize = ftell(file);//check byte size of file
                    fread(filebuf, bytesize, 1, file);//read entire file into filebuf[300]
                    strcpy(Spdu.Cdata, filebuf);//copy read file into contant data of sent PDU
                    write(new_TCP, &Spdu, sizeof(Spdu));
                }
                else{
                    printf("file does not exist\n");
                }
                
            }
    }
}//end of while loop

}

void extractData(char* data){
    int n = strlen(data);
    //printf("INSIDE extractData, size of data (%s): %d\n", data, n);
    int i;
    for(i = 0; i < n; i++){
        if(data[i] == '~'){
            data[i] = '\0';
        }
    }

}
