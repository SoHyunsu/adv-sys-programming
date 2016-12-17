#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 3000

#define MAX_CLIENT 1024
#define MAX_DATA 1024

void sendMessage(int s, char* buf);

struct sockaddr_in servaddr;
int addrlen = sizeof(servaddr); //서버 주소의 size를 저장

int main(int argc, char *argv[]) {
	
	int s; //socket
	int nbyte;
	char buf_send[MAX_DATA+1], buf_recv[MAX_DATA+1];
	FILE *inFile;
	FILE *outFile; 
	FILE *stream; 

	if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
       perror("socket fail");
       exit(0);
	}
	   
	memset(&servaddr, 0, addrlen); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(IP);
	servaddr.sin_port = htons(PORT);

	if((stream = fopen(argv[3] + ".txt", "r")) == NULL) {
        printf("Error not File");
        exit(1);
    }
	if((stream = fopen(argv[3] + "out.txt", "wt")) == NULL) {
        printf("Error not File");
        exit(1);
    }
   
	sendMessage(s, "$"); 
    while(!feof(stream))
    {
        buf_send[0] = '\0'; 
		buf_recv[0] = '\0'; 
        fgets(buf_send, MAX_DATA, stream); 
		sendMessage(s, buf_send);
        if((nbyte = recvfrom(s, buf_recv, MAX_DATA, 0, (struct sockaddr *)&servaddr, &addrlen)) > 0) {
			fputs(buf_recv, outFile);
        }
    }
	sendMessage(s, "@"); 
    fclose(stream); //stream close
	close(s); //socket close
	return 0;
}

void sendMessage(int s, char* buf) {
   if((sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
       perror("send to fail");
       exit(0);
   }
}

