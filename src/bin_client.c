#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <math.h>

#define SERVERPORT "4950"    // the port users will be connecting to
#define MAXBUFLEN 100

#define WEIGHTS 5
#define AVGS 30
#define DIFF  		0.01
#define W_POWER 	0.27
#define W_REMOTE 	0.40
#define W_PAMPHLET 	0.22
#define W_PAPER	 	0.06
#define W_COMP	 	0.50
#define ERROR		0.01
#define STABLE_INTERVAL 25


float avg = 0;
float W_MAX = 0;
float W_EMPTY = 3.52;

char* picked(FILE* file){

	char buff[100];
	float weights[WEIGHTS];
	float avgs[AVGS];
	int cb_i = 0;
	int avg_i = 0;
	short stable = 1;
	float prev_weight = 0;


	printf("Reading...\n");

	while(1) { 			// Main loop
		float weight;
		fgets(buff, 100, file);
		weight = atof(buff);
		if(weight != 0.0) {	// Read in valid weight

			weights[cb_i] = weight;	 // Add weight to circular buffer
			if(cb_i==WEIGHTS-1) {
				cb_i=0;
			} else {
				cb_i++;
			}
			int i;


			avg=0; 		// Find the average of all weights in the buffer
			for(i=0; i < WEIGHTS; i++) {
				avg += weights[i];
			}
			avg = avg/WEIGHTS;


			avgs[avg_i] = avg;	// Add this weight to the averages

			for(i = 0; i < AVGS ; i++){  // Wait for the weight to stabalize

				if ( i == 0 ) { 
					if ( fabs(avgs[AVGS-1] - avgs[i]) < DIFF) {
						stable = 1;
					} else{
						stable = 0;
						break;
					}
				}
				else {
					if ( fabs(avgs[i-1] - avgs[i]) < DIFF) {
						stable = 1;
					} else{
						stable = 0;
						break;
					}
				}
			}
			
			int t;
			if ( !stable ) { 	// Wait for STABLE_INTERVAL stable averages
				t=0;
			}
			else{
				if(t >= STABLE_INTERVAL) {
					if (t == STABLE_INTERVAL) {
						float percent_full = 0;
						if ( prev_weight - avg  < -ERROR ) {
							printf("\n\nAdded Weight! (%f lb)\n\n", avg-prev_weight);
							W_MAX = avg;
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
						} else if( prev_weight - avg < ERROR) {
						} else if( prev_weight - avg < W_PAPER){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Paper picked (%d Percent Full)\n",(int)percent_full);
							return "Paper";
						} else if( prev_weight - avg < W_PAMPHLET){
							percent_full = (avg - W_EMPTY)*100/ (W_MAX-W_EMPTY);
							printf("Pamphlet picked (%d Percent Full)\n",(int)percent_full);
							return "Pamphlet";
						} else if( prev_weight - avg < W_POWER){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Power cable picked (%d Percent Full)\n",(int)percent_full);
							return "Power cable";
						} else if( prev_weight - avg < W_REMOTE){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Remote picked (%d Percent Full)\n",(int)percent_full);
							return "Remote";
						} else if( prev_weight - avg < W_COMP){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Comp cable  picked (%d Percent Full)\n",(int)percent_full);
							return "Component Cable";
						} else {
						}
					}
					prev_weight = avg;
					t++;
				}
				else {
					t++;
				}
			}


			if(avg_i==AVGS-1) { 	// Check circular buffer
				avg_i=0;
			} else {
				avg_i++;
			}

		}

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

	//printf("listener: got packet from %s\n",
	//		inet_ntop(their_addr.ss_family,
	//			get_in_addr((struct sockaddr *)&their_addr),
	//			s, sizeof s));
	//printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	//printf("\n%s > %s", ip, buf);
	//return buf;

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

int main(int argc, const char *argv[])
{
	FILE *file = fopen("/dev/ttyUSB0","r");
	if(!file){
		perror("Unable to open device");
		exit(-1);
	}

	char * buf = (char*)malloc(sizeof(char)*MAXBUFLEN);
	while(1) {
		get_msg(argv[1], buf);
		printf("Pick a %s\n",buf);

		char *message = picked(file);
		send_msg(argv[1], message);
	}

//	close(file);	
	return 0;
}
