#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "comm.h"

#define SERVERPORT "4950"
#define MAXBUFLEN 100
#define QUEUELEN 5
#define PORT 6699

// Copies string in buffer buff into next sent network packet
void set_mesg_send(struct network_data *data, char* buff){
	sem_wait(&data->lock_send);
	strcpy(data->msg_send, buff);
	sem_post(&data->lock_send);
}

// Copies string in recieved network packet to buffer buff
void get_mesg_recv(struct network_data *data, char* buff){
	sem_wait(&data->lock_recv);
	strcpy(buff,data->msg_recv);
	sem_post(&data->lock_recv);
}

int network_init(struct network_data *data){
	memset(&data->msg_recv,'\0',sizeof(data->msg_recv));
	memset(&data->msg_send,'\0',sizeof(data->msg_send));
	sem_init(&data->lock_send,1,1);
	sem_init(&data->lock_recv,1,1);

	return 1;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_msg(const char* ip, char* buf)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	//char buf[MAXBUFLEN];
	socklen_t addr_len; char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	//printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
					(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[numbytes] = '\0';

	close(sockfd);

	return 0;
}


int send_msg(const char *ip, char *message)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(ip, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, message, strlen(message), 0,
					p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	//printf("talker: sent %d bytes to %s\n", numbytes, ip);
	close(sockfd);

	return 0;
}

void *server_daemon(void *arg){
	struct network_data *data = (struct network_data*) arg;

	struct sockaddr_in sad; 	// hold server's address
	struct sockaddr_in cad; 	// hold client's address
	int sd, sd2; 			// socket descriptors
	int port;  			// protocol port number
	socklen_t alen; 		// length of address
	unsigned int in_index; 		// index of incoming message

	memset((char *) &sad, 0, sizeof(sad)); 	// Initialize SAD to zero
	sad.sin_family = AF_INET; 	// Internet address type
	sad.sin_addr.s_addr = INADDR_ANY; 	// Local IP address

	if (PORT > 0)
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"SERVER_DAEMON: bad port number %d", PORT);
		exit(-1);
	}

	// initialize socket ID
	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( sd < 0 ) {
		perror("SERVER_DAEMON: socket creation failed");
	}

	// assign IP/port number to socket where connections come in
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		perror("SERVER_DAEMON: bind failed");
		exit(-1);
	}

	// set up socket to receive incoming connections
	
	if (listen(sd, QUEUELEN) < 1) {
		perror("SERVER_DAEMON: listen failed");
		exit(-1);
	}

	while (1) {
		alen = sizeof(cad);
		
		if ( (sd2 = accept(sd, (struct sockaddr*)&cad, &alen)) < 0) {
			perror("SERVER_DAEMON: accept failed");
			exit(-1);
		}

		sem_wait(&data->lock_recv);
		if ( (recv(sd2, &data->msg_recv, MAXBUFLEN, 0) < 0)) {
				perror("SERVER_DAEMON: could not recvfrom: ");
				exit(-1);
		}
		data->msg_recv[1000] = '\0';
		sem_post(&data->lock_recv);

		// HANDLE IN_MSG BUFFER
		
		sem_wait(&data->lock_send);
		if(send(sd2, &data->msg_send, strlen(data->msg_send)+1, 0) < 0) {
			perror("SERVER_DAEMON: could not send: ");
			exit(-1);
		}
		memset(&data->msg_send, '\0', MAXBUFLEN);
		sem_post(&data->lock_send);

		close(sd2);
		
	}

	return 0;
}
