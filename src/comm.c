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
#include "scan.h"
#include "AKS_errors.h"
#include "configuration.h"

#define SERVERPORT "4950"
#define MAXBUFLEN 512
#define QUEUELEN 5

extern char * config_file_path;

char client_ip[INET_ADDRSTRLEN];

// Copies string in buffer buff into next sent network packet
void set_msg_send(struct network_data *data, char* buff){
	sem_wait(&data->lock_send);
	strcpy(data->msg_send, buff);
	sem_post(&data->lock_send);
}

// Copies string in recieved network packet to buffer buff
void get_msg_recv(struct network_data *data, char* buff){
	sem_wait(&data->lock_recv);
	strcpy(buff,data->msg_recv);
	memset(data->msg_recv,'\0',sizeof(char)*data->size);
	sem_post(&data->lock_recv);
}

int network_init(struct network_data *data, int size){
	data->size = size;
	data->msg_recv = malloc(sizeof(char)*size);
	data->msg_send = malloc(sizeof(char)*size);
	memset(data->msg_recv,'\0',sizeof(char)*size);
	memset(data->msg_send,'\0',sizeof(char)*size);

	sem_init(&data->lock_send,1,1);
	sem_init(&data->lock_recv,1,1);

	pthread_attr_init(&data->thread_attr);
	if( pthread_create(&data->thread_id, &data->thread_attr, server_daemon, data) < 0) { 
		perror("Unable to create server daemon thread");
//		exit(-1);
	}


	pthread_attr_init(&data->sender_attr);
	if(pthread_create(&data->sender_id, &data->sender_attr, sender, data) < 0) {
		perror("Unable to create sender thread");
	}


	return 1;
}

char* handle_message(char *msg) {
	char* buff = malloc(sizeof(char)*MAXBUFLEN);
	switch(msg[0]){
		case '1':	 // Let the main thread handle this message
			return "\0"; 
		case '2':	// Return the current configuration
			write_configuration_str(config_file_path, msg+2);
			return "2 Success";
		case '3':	// Return the current configuration
			read_configuration_str(config_file_path, buff);
			return buff;
		default:
			printf("Message type %c undefined\n",msg[0]);
			return "Message undefined";
	}
}

void *server_daemon(void* arg) {

	struct network_data *data = (struct network_data*) arg;
	struct sockaddr_in sad;  // structure to hold server's address  
	struct sockaddr_in cad;  // structure to hold client's address  
	int sd = 0; 
	int sd2 = 0;               // socket descriptors                        
	int port = atoi(SERVERPORT);                            // protocol port number            
	socklen_t alen;          // length of address                   
	unsigned int in_index;   // index to incoming message buffer
	char buff[data->size];

	// prepare address data structure

	memset((char *)&sad,0,sizeof(sad)); // zero out sockaddr structure      
	sad.sin_family = AF_INET;                 // set family to Internet             
	sad.sin_addr.s_addr = INADDR_ANY;   // set the local IP address 

	if (port > 0)
		// test for illegal value       
		sad.sin_port = htons((u_short)port);
	else {
		// print error message and exit 
		fprintf(stderr,"ECHOD: bad port number %s\n", port);
		exit(-1);
	}

	// create socket 

	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0) {
		perror("ECHOD: socket creation failed");
		exit(-1);
	}

	// assign IP/port number to socket where connections come in 

	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		perror("ECHOD: bind failed");
		exit(-1);
	}

	// set up socket to receive incomming connections 

	if (listen(sd, QUEUELEN) < 0) {
		perror("ECHOD: listen failed");
		exit(-1);
	}

	// main server loop - accept and handle requests 
	


	while (1) {
		alen = sizeof(cad);

		printf("CHECKING FOR PACKET\n");
		if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			perror("ECHOD: accept failed\n");
			exit(-1);
		}
		printf("HERE at accept\n");

		inet_ntop(AF_INET, &(cad.sin_addr), client_ip, INET_ADDRSTRLEN);

		printf("Client IP: %s\n", client_ip);

		// receive the string sent by client
		if (recv(sd2, &buff, data->size, 0) < 0) {
			perror("Could not recvfrom: ");
			exit(-1);
		}
		buff[data->size] = '\0';
		printf("String recieved : %s\n", buff);

		printf("Waiting...\n");
		sem_wait(&data->lock_recv);
		printf("Acquired...\n");

		strcpy(data->msg_recv, buff);

		sem_post(&data->lock_recv);
		printf("Copied packet into msg_recv\n");

		set_msg_send(data, handle_message(buff));
		printf("Handled buffer\n");

		memset(&buff, '\0', data->size);
		close(sd2);
	}
}

void *sender(void *arg) {
	struct network_data *data = (struct network_data*) arg;

	printf("Initialized SENDER thread\n");
	while(1) {
		sem_wait(&data->lock_send);
		if(data->msg_send[0] != '\0') {
			printf("SENDING MESSAGE\n");
			// send the received string back to client
			olp_send_recv(client_ip, atoi(SERVERPORT), data->msg_send); 
			memset(data->msg_send,'\0',sizeof(char)*data->size);
			printf("message reset%d\n",data->msg_send);
		}
		sem_post(&data->lock_send);
	}
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
	int port = atoi(SERVERPORT);
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

	printf("talker: sent %d bytes to %s\n", numbytes, ip);
	close(sockfd);

	return 0;
}

char * olp_send_recv(const char* host, int port, char* message) {
	struct	hostent	 *ptrh;	 // pointer to a host table entry	
	struct	sockaddr_in sad; // structure to hold an IP address	

	int	sd;		                 // socket descriptor			
	char  in_msg[MAXBUFLEN]; // buffer for incoming message

	unsigned int in_index;     // index to incoming message buffer

	memset((char *)&sad,0,sizeof(sad)); // zero out sockaddr structure	
	sad.sin_family = AF_INET;	          // set family to Internet	

	// verify usage


	if (port > 0)	
		// test for legal value		
		sad.sin_port = htons((u_short)port);
	else {				
		// print error message and exit	
		printf("ECHOREQ: bad port number %d\n", port);
//		exit(-1);
	}

	// convert host name to equivalent IP address and copy to sad 

	ptrh = gethostbyname(host);

	if ( ((char *)ptrh) == NULL ) {
		printf("ECHOREQ: invalid host: %s\n", host);
//		exit(-1);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	// create socket 

	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0) {
		printf("ECHOREQ: socket creation failed\n");
		//exit(-1);
	}

	// connect the socket to the specified server 

	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		perror("ECHOREQ: connect failed");
		close(sd);
		return "FAILED";
		//exit(-1);
	}

	// send message to server
	if (send(sd, message, strlen(message), 0) < 0) {
		perror("Failed to send message to server: ");
		//exit(-1);
	}

	// receive message echoed back by server
//	if (recv(sd, &in_msg, MAXBUFLEN, 0) < 0) {
//		perror("Failed to send message to server: ");
//		exit(-1);
//	}
//	in_msg[MAXBUFLEN] = '\0';

//	printf("ECHOREQ: from server= %s\n", in_msg);
//
	shutdown(sd,2);

	// close the socket   
	close(sd);

	printf("Done sending\n");
	// terminate the client program gracefully 
	return "DONE";
}
