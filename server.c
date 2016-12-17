#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENT   20
#define DEFAULT_PORT 3000
#define MAX_EVENTS   10000

struct {
	int  cli_sockfd;  
	char cli_ip[20];  
} g_client[MAX_CLIENT];

int g_svr_sockfd;
int g_svr_port;
int g_epoll_fd;
struct epoll_event g_events[MAX_EVENTS];

void epoll_cli_add(int cli_fd);   
void userpool_add(int cli_fd, char *cli_ip);
void userpool_delete(int cli_fd);
void server_process();
void client_recv(int event_fd);
void end_server(int sig);

int main(int argc, char *argv[]){
	
	struct sockaddr_in serv_addr;
	struct epoll_event events;
	int i;

	g_svr_port = DEFAULT_PORT;
	for (i = 0; i < MAX_CLIENT; i++){
		g_client[i].cli_sockfd = -1;
	}

	// socket
	if ((g_svr_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Can't open stream socket \n");
		exit(0);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(g_svr_port);

	int nSocketOpt = 1;
	if (setsockopt(g_svr_sockfd, SOL_SOCKET, SO_REUSEADDR, &nSocketOpt, sizeof(nSocketOpt)) < 0){
		printf("Can't set reuse address\n");
		close(g_svr_sockfd);
		exit(0);
	}

	if (bind(g_svr_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("Can't bind local address\n");
		close(g_svr_sockfd);
		exit(0);
	}

	listen(g_svr_sockfd, 15);

	printf("Now Server listening on port %d\n", g_svr_port);

	// epoll
	g_epoll_fd = epoll_create(MAX_EVENTS);
	if (g_epoll_fd < 0){
		printf("Epoll create Fails.\n");
		close(g_svr_sockfd);
		exit(0);
	}
	printf("epoll creation success\n");

	events.events = EPOLLIN;
	events.data.fd = g_svr_sockfd;

	if (epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, g_svr_sockfd, &events) < 0){
		printf("[ETEST] Epoll control fails.\n");
		close(g_svr_sockfd);
		close(g_epoll_fd);
		exit(0);
	}

	printf("epoll events set success for server\n");

	while (1){
		server_process();  
	}
}

void epoll_cli_add(int cli_fd){

	struct epoll_event events;

	events.events = EPOLLIN;
	events.data.fd = cli_fd;

	if (epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, cli_fd, &events) < 0){
		printf("Epoll control fails.in epoll_cli_add\n");
	}

}

void userpool_add(int cli_fd, char *cli_ip){
	int i;

	for (i = 0; i < MAX_CLIENT; i++){
		if (g_client[i].cli_sockfd == -1) {
			break;
		}
	}
	if (i >= MAX_CLIENT){
		close(cli_fd);
	}
	g_client[i].cli_sockfd = cli_fd;
	memset(&g_client[i].cli_ip[0], 0, 20);
	strcpy(&g_client[i].cli_ip[0], cli_ip);
}

void userpool_delete(int cli_fd){
	int i;

	for (i = 0; i < MAX_CLIENT; i++){
		if (g_client[i].cli_sockfd == cli_fd){
			g_client[i].cli_sockfd = -1;
			break;
		}
	}
}

void server_process(){
	struct sockaddr_in cli_addr;
	int i, nfds;
	int cli_sockfd;
	int cli_len = sizeof(cli_addr);

	nfds = epoll_wait(g_epoll_fd, g_events, MAX_EVENTS, 100); 

	// no event , no work 
	if (nfds == 0){
		return;
	}
	// return but this is epoll wait error 
	if (nfds < 0){
		printf("[ETEST] epoll wait error\n");
		return; 
	}

	for (i = 0; i < nfds; i++){
		if (g_events[i].data.fd == g_svr_sockfd){
			cli_sockfd = accept(g_svr_sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_len);
			// accept error	
			if (cli_sockfd < 0){
			}
			else{
				printf("New client connected. fd:%d,ip:%s\n", cli_sockfd, inet_ntoa(cli_addr.sin_addr));
				userpool_add(cli_sockfd, inet_ntoa(cli_addr.sin_addr));
				epoll_cli_add(cli_sockfd);
			}
			// next fd 
			continue; 
		}
		client_recv(g_events[i].data.fd);
	} 
}


void client_recv(int event_fd){
	char r_buffer[1024];
	int len;
	int i;

	len = recv(event_fd, r_buffer, 1024, 0);
	if (len < 0 || len == 0){
		userpool_delete(event_fd);
		close(event_fd);
		return;
	}

	len = strlen(r_buffer);
	for (i = 0; i < MAX_CLIENT; i++){
		if (g_client[i].cli_sockfd != -1){
			len = send(g_client[i].cli_sockfd, r_buffer, len, 0);
		}
	}
}

void end_server(int sig){
	close(g_svr_sockfd);
	printf("Server closed by signal %d\n", sig);
	exit(0);
}
