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

#define AWS_SERVER_PORT "26951"
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

void printReceivedMonitor(char* in){
	//Request types:
	//1: search
	//2|3: prefix and suffix
	//Message format:
	//Output format:
	//NOT FOUND: 0&INPUT
	//SEARCH AND FOUND SIMILAR: 1&INPUT&VALUE&WORD&DEFINITION
	//SEARCH AND DIDN'T FIND SIMILAR: 1&INPUT&VALUE
	//DIDNT FIND BUT FOUND SIMILAR: 1&&INPUT&WORD&DEFINITION
	//PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn
	if(in[0] == '0'){
		printf("Found no matches for <");
		int i = 2;
		while(in[i] != '\0'){
			printf("%c",in[i]);
			i++;
		}
		printf(">\n");
	}
	else if(in[0] == '1'){
		if((in[1] == '&')&&(in[2] == '&')){ //Special case if search didn't find the exact word but found at least one similar word
			printf("Found no matches for <");
			int i = 3;
			while(in[i] != '&'){
				printf("%c",in[i]);
				i++;
			}
			i++;
			printf(">\nOne edit distance match is <");
			while(in[i] != '&'){
				printf("%c",in[i]);
				i++;
			}
			i++;
			printf(">\n<");
			while(in[i] != '\0'){
				printf("%c",in[i]);
				i++;
			}
			printf(">\n");
		}
		else{
			printf("Found a match for <");
			int i = 2;
			while(in[i] != '&'){
				printf("%c",in[i]);
				i++;
			}
			i++;
			printf(">:\n<");
			while(in[i] != '\0'){
				if(in[i] == '&'){
					printf(">\nOne edit distance match is <");
					i++;
					while(in[i] != '\0'){
						if(in[i] == '&'){
							printf(">:\n<");
						}
						else{
							printf("%c",in[i]);
						}
						i++;
					}
					while(in[i] != '\0'){
						printf("%c",in[i]);
					}	
					printf(">\n");
					return;
				}
				else{
					printf("%c",in[i]);
					i++;
				}
			}
			printf("\nNo one edit distance match found\n");
		}
	}
	else if((in[0] == '2')||(in[0] == '3')){
		int n = 0;
		int i = 2;
		int power = 0;
		while(in[i] != '&'){
			power ++;
			i++;
		}
		i++;
		int i_temp;
		for(i_temp = 2; i_temp < 2 + power; i_temp ++){
			n += (int)(in[i_temp]-0x30)*pow(10,power-i_temp+1);
		}

		printf("Found %d matches for <",n);
		while(in[i] != '&'){
			printf("%c",in[i]);
			i++;
		}
		i++;
		printf(">:\n<");
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



//Returns 0 if search
//Returns 1 if prefix
//Returns 2 if suffix
//Returns -1 otherwise
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
			perror("monitor: socket");
			continue;
		}
		//connect()
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("monitor: connect");
			continue;
		}
		break;
	}

	if(p == NULL){
		fprintf(stderr, "monitor: failed to connect\n");
		return(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), ip, sizeof(ip));
	//printf("monitor: connecting to %s\n",ip);
	printf("The monitor is up and running \n");

	//Get local port
	// struct sockaddr_in local;
	// socklen_t l = sizeof(local);
	// getsockname(sockfd,(struct sockaddr*)&local,(socklen_t *)&l);
	// printf("local port: %d\n", (int)ntohs(local.sin_port));

	freeaddrinfo(res);
	//End of block

	while(1){
		if ((recv(sockfd, inputBuffer, DATA_SIZE, 0)) == -1) {
	    	perror("recv");
	    	exit(1);
	    }
	    else{
	    	printf("Received from AWS:%s\n",inputBuffer);
	    	printReceivedMonitor(inputBuffer);
	    }
	}

	close(sockfd);

	return 0;
}