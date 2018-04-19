#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <cmath>

#define AWS_SERVER_PORT "25951"
#define AWS_SERVER_IP "localhost"

#define DATA_SIZE 1000

// //For reference
// struct sockaddr{
// 	unsigned short sa_family; //AF_INET for IPv4 or AF_INET6 for IPv6
// 	char sa_data[14]; //protocol address
// }

// struct addrinfo{ 
// 	int ai_flags;
// 	int ai_family; //AF_INET for IPv4, AF_INET6 for IPv6 or AF_UNSPEC for unspecified
// 	int ai_socktype; //SOCK_DGRAM for connectionless datagram socket or SOCK_STREAM for reliable stream socket 
// 	int ai_protocol;
// 	size_t ai_addrlen;
// 	struct sockaddr* ai_addr;
// 	char* ai_canonname;

// 	struct addrinfo* ai_next;
// };

//Taken from Beej's network programming guide section 6.2 client.c
//Returns sockaddr_in or sockaddr_in6 type depending on the sa_family parameter 
void *get_in_addr(struct sockaddr* sa){
	if(sa->sa_family == AF_INET){
		return(&(((struct sockaddr_in*)sa)->sin_addr));
	}
	return(&(((struct sockaddr_in6*)sa)->sin6_addr));
}

//Taken from Beej's network programming guide section 7.3 
int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

void printReceived(char requestType, char* in, const char* word){
	//Request types:
	//0: search
	//1: prefix and suffix
	//Message format:
	//If not found: '&\0'
	//If search: 'VALUE\0'
	//If prefix or suffix: 'n&VALUE1&VALUE2&VALUE3...&VALUEn\0'
	if(in[0] == '&'){
		printf("Found no matches for <%s>\n", word);
	}
	else if(requestType == '1'){
		printf("Found a match for <%s>:\n<",word);
		int i = 0;
		while(in[i] != '\0'){
			printf("%c",in[i]);
			i++;
		}
		printf(">\n");
	}
	else if((requestType == '2')||(requestType == '3')){
		int n = 0;
		int i = 0;
		int power = 0;
		while(in[i] != '&'){
			power ++;
			i++;
		}
		i++;
		int i_temp;
		for(i_temp = 0; i_temp < power; i_temp ++){
			n += (int)(in[i_temp]-0x30)*pow(10,power-i_temp-1);
		}
		printf("Found <%d> results for <%s>:\n<", n, word);
		while(in[i] != '\0'){
			if(in[i] == '&'){
				printf(">\n<");
			}
			else{
				printf("%c",in[i]);
			}
			i++;
		}
		printf(">\n");
	}
	else{
		printf("Print received error\n");
	}
}

//Returns 1 if search
//Returns 2 if prefix
//Returns 3 if suffix
//Returns 0 otherwise
char getRequestType(const char* r){
	char search[] = "search";
	char prefix[] = "prefix";
	char suffix[] = "suffix";

	//compare with search
	int i = 0; 
	if(strcmp(r,search) == 0){
		return('1');
	}
	else if(strcmp(r,prefix) == 0){
		return('2');
	}
	else if(strcmp(r,suffix) == 0){
		return('3');
	}
	else{
		return('0');
	}
}


int main(int argc, char const *argv[])
{
	if(argc < 3){
		printf("Requires two arguments \n");
		return(1);
	}
	else if(argc > 3){
		printf("Too many arguments \n");
		return(1);
	}

	char requestType;
	if((requestType = getRequestType(argv[1])) == '0'){
		printf("Invalid request");
	}


	char ip[INET6_ADDRSTRLEN];
	struct addrinfo hints,*res,*p; //hints is a struct I am going to load with relevant information 
	int sockfd; //socket descriptor
	char outputBuffer[DATA_SIZE];
	char inputBuffer[DATA_SIZE];
	int status;

	//Following is taken from Beej's network programming guide section 5 showip.c
	memset(&hints,0,sizeof(hints)); //Clear the memory at the address of hints
	hints.ai_family = AF_UNSPEC; //the IP protocol is unspecified 
	hints.ai_socktype = SOCK_STREAM; //we need a TCP socket so using SOCK_STREAM

	if((status = getaddrinfo(AWS_SERVER_IP, AWS_SERVER_PORT, &hints, &res)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return(1);
	} 
	//End of block


	//Following is taken from Beej's network programming guide section 6.2 client.c
	for(p = res; p != NULL; p = p->ai_next){
		//socket()
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}
		//connect()
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if(p == NULL){
		fprintf(stderr, "client: failed to connect\n");
		return(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), ip, sizeof(ip));
	//printf("client: connecting to %s\n",ip);
	printf("The client is up and running \n");

	//Get local port
	// struct sockaddr_in local;
	// socklen_t l = sizeof(local);
	// getsockname(sockfd,(struct sockaddr*)&local,(socklen_t *)&l);
	// printf("local port: %d\n", (int)ntohs(local.sin_port));

	freeaddrinfo(res);
	//End of block

	//Load output buffer
	outputBuffer[0] = requestType;
	outputBuffer[1] = 0x26; //seperator character. I used &
	int i = 0;
	int j = 2;
	while(argv[2][i] != '\0'){
		outputBuffer[j] = argv[2][i];
		i++;
		j++;
	}
	outputBuffer[j] = '\0';

	//Send request
	int len = j+1;
	if(sendall(sockfd,outputBuffer,&len) == -1){
		perror("send");
	}
	else{
		printf("The client sent %s and %s to AWS.\n",argv[2],argv[1]);
	}

	if ((recv(sockfd, inputBuffer, DATA_SIZE, 0)) == -1) {
    	perror("recv");
    	exit(1);
    }

    //printf("Received from AWS:%s\n",inputBuffer);
    printReceived(requestType, inputBuffer,argv[2]);

	close(sockfd);

	return 0;
}