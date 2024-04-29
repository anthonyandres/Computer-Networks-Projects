#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

struct pdu{
	char type;
	char data[100];
    char registerPeer[11];
    char registerContent[11];
    char registerPort[15];
    char registerIP[25];
    char Cdata[9999]; //subject to change, might dynamically allocate
    //https://www.programiz.com/c-programming/examples/structure-dynamic-memory-allocation
    int numPeers;
    char peerName[5][10];
    char fileName[5][10];
    int portNum;

};

struct peer{
    char name[20];
    char contentName[20];
    char port[20];
    char ipAddr[20];
    //struct peer *next;
};

void extractData(char* data);

int main(int argc, char *argv[])
{
/*
------------------------------------------------------------------------
 UDP SERVER
------------------------------------------------------------------------
*/
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     UDP, type;        /* socket descriptor and socket type    */
	int 	port=3000;
                                                                                

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
        UDP = socket(AF_INET, SOCK_DGRAM, 0);
        if (UDP < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(UDP, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(UDP, 5);	
	alen = sizeof(fsin);

    //totalPeers and peerCount is to keep track of how many total registered peers
    //there are as well as keeping track of which peer we are on
    //when we iterate through the list of peers: peerList[3]
    int peerCount = 0;
    int totalPeers = 0; 
    struct pdu Rpdu;
    struct peer peerList[3]; //index server can register up to 3 peers
    struct peer peerListBuf[3];
    bool dupe;

    while(1){
        //recvfrom is stopping, and code will not run until data received
        //ALL INDEX <=> PEER COMMUNICATION IS UDP
        printf("waiting...\n");
        recvfrom(UDP, &Rpdu, sizeof(Rpdu), 0, (struct sockaddr *)&fsin, &alen);

        //Peer wants to download
        if(Rpdu.type == 'S'){
            printf("\n-----Download Request from peer-----\n");
            int i;
            char buffer[20];
            char dlrqPeer[20];
            char dlrqContent[20];
            struct pdu Spdu;
            memset(buffer, 0, sizeof(buffer));
            
            strncpy(buffer, Rpdu.data + 0, 10);
            extractData(buffer);
            strcpy(dlrqPeer, buffer);
            memset(buffer, 0, sizeof(buffer));

            strncpy(buffer, Rpdu.data + 10, 10);
            extractData(buffer);
            strcpy(dlrqContent, buffer);
            memset(buffer, 0, sizeof(buffer));

            //at this point both the download request peer name is in dlrqPeer
            //and the download request content name is in dlrqContent

            for(i = 0; i < totalPeers; i++){
                if(strcmp(dlrqPeer, peerList[i].name) == 0 && strcmp(dlrqContent, peerList[i].contentName) == 0){
                //if requested information exists put ip and port in S type PDU
                    char ipAddrBuf[20], portBuf[20];
                    int n,j;
                    strcpy(ipAddrBuf, peerList[i].ipAddr);
                    strcpy(portBuf, peerList[i].port);
                    
                    int word1 = strlen(ipAddrBuf);
                    int word2 = strlen(portBuf);

                    //pad words until both 20 char long (peer will extract after)
                    for(j = 0; j < (sizeof(ipAddrBuf) - (word1)); j++){
                        strcat(ipAddrBuf, "~");
                    }
                    for(j = 0; j < (sizeof(portBuf) - (word2)); j++){
                        strcat(portBuf, "~");
                    }

                    strcpy(Spdu.data, portBuf);
                    strcat(Spdu.data, ipAddrBuf);
                    Spdu.type = 'S';
                    (void) sendto(UDP, &Spdu, sizeof(Spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
                }
                else{
                    //send E type PDU
                    Spdu.type = 'E';
                    (void) sendto(UDP, &Spdu, sizeof(Spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
                }
            }

        }

        //Peer wants to register
        if(Rpdu.type == 'R'){
            printf("\n----------Registering Peer----------\n");
            char buffer[20];
            memset(buffer,0,sizeof(buffer));   

            //strncpy(buffer, Rpdu.data + 0, 10);      //put "name", index(0-9), into buffer
            //extractData(buffer);                        //extractData() will remove any '~' 
            
            strcpy(buffer, Rpdu.registerPeer);
            int i;
            dupe = false;
            for(i = 0; i < totalPeers; i++){ //check for duplicate names
                if(strcmp(buffer, peerList[i].name) == 0){
                    dupe = true;
                }
            }
            if(dupe){
                    printf("!error detected, duplicate names\nnotifying peer\n");
                    struct pdu Spdu;
                    Spdu.type = 'E';
                    (void) sendto(UDP, &Spdu, sizeof(Spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
                    continue; //begin from beginning of while loop again
                }

            // strcpy(peerList[peerCount].name, buffer);   //copy cleaned buffer into struct.name
            // memset(buffer,0,sizeof(buffer));            //clear buffer

            // strncpy(buffer, Rpdu.data + 10, 10);           //put "contentName", index(10-19), into buffer
            // extractData(buffer);
            // strcpy(peerList[peerCount].contentName, buffer);
            // memset(buffer,0,sizeof(buffer));

            // strncpy(buffer, Rpdu.data + 20, 10);
            // extractData(buffer);
            // strcpy(peerList[peerCount].port, buffer);
            // memset(buffer,0,sizeof(buffer));

            // strncpy(peerList[peerCount].ipAddr, Rpdu.data + 30, 15);

            strcpy(peerList[peerCount].name, Rpdu.registerPeer);
            strcpy(peerList[peerCount].contentName, Rpdu.registerContent);
            strcpy(peerList[peerCount].port, Rpdu.registerPort);
            strcpy(peerList[peerCount].ipAddr, Rpdu.registerIP);

    //^ the above will receive the data as a string with misc '~' and remove those ~ accordingly
    // then it will store the cleaned data within a peer struct correctly
            printf("final name:        %s\n", peerList[peerCount].name);
            printf("final contentName: %s\n", peerList[peerCount].contentName);
            printf("final port:        %s\n", peerList[peerCount].port);
            printf("final ipAddr:      %s\n", peerList[peerCount].ipAddr);
            printf("------------------------------------\n\n");
            
            //printf("%c\n", Rpdu.type);
            //printf("%s\n", Rpdu.data);
            struct pdu Spdu;
            Spdu.type = 'A';
            (void) sendto(UDP, &Spdu, sizeof(Spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
            peerCount++;
            totalPeers++;
        }

        //peer wants to De-register
        if(Rpdu.type == 'T'){
            printf("peer wants to delete: %s\n", Rpdu.data);
            int i;
            int n;
            int subtract = 0;
            struct pdu Spdu;

            for(i = 0, n = 0; i < totalPeers; i++){ //copy the non deregistered items into a buffer array of structs
                if(strcmp(peerList[i].name, Rpdu.data) == 0){
                    //printf("de-registering i = %d\n", i);
                    //printf("de-registering this: %s\n", peerList[i].name);
                    subtract++;
                    continue;
                }
                else{
                    peerListBuf[n] = peerList[i];
                    strcpy(peerListBuf[i].name, peerList[i].name);
                    strcpy(peerListBuf[i].contentName, peerList[i].contentName);
                    strcpy(peerListBuf[i].ipAddr, peerList[i].ipAddr);
                    strcpy(peerListBuf[i].port, peerList[i].port);
                    //printf("keeping i = %d\n", i);
                    //printf("keeping this: %s\n", peerList[i].name);
                    n++;
                }
            }

            for(i = 0; i < totalPeers-1; i++){ //copy the new array to original array
                //printf("newArray i = %d\n", i);
                strcpy(peerList[i].name, peerListBuf[i].name);
                strcpy(peerList[i].contentName, peerListBuf[i].contentName);
                strcpy(peerList[i].ipAddr, peerListBuf[i].ipAddr);
                strcpy(peerList[i].port, peerListBuf[i].port);
                //printf("new array: %s\n", peerListBuf[i].name);
            }
            totalPeers = totalPeers - subtract;
            peerCount--;
            Spdu.type = 'A';
            (void) sendto(UDP, &Spdu, sizeof(Spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
        }

        //peer wants a list of Registered Content
        if(Rpdu.type == 'O'){
            struct pdu Spdu;
            Spdu.type = 'A';
            int i;
            for(i = 0; i < totalPeers; i++){
                strcpy(Spdu.peerName[i], peerList[i].name);
                strcpy(Spdu.fileName[i], peerList[i].contentName);
            }
            Spdu.numPeers = totalPeers;
            (void) sendto(UDP, &Spdu, sizeof(Spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
        }


    }
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

