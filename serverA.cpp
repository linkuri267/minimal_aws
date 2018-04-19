#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "backend.cpp"
#include <iostream>

#define LOCAL_IP "localhost"
#define AWS_SERVER_IP "localhost"

#define AWS_SERVER_PORT "24951"
#define LOCAL_PORT "21951"

#define MAXBUFLEN 1000

#define FILE_NAME "backendA.txt"

//Following function taken from Beej listener.c

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char const *argv[])
{
	//load dictionary and tries
	std::unordered_map <std::string, std::string> myDictionary;
	prefixTree myPrefixTree = prefixTree();
	startup(FILE_NAME,myDictionary,myPrefixTree);

	//Following taken from Beej listener.c
	int sockfd_in;
    struct addrinfo hints_in, *servinfo_in, *p_in;
    int rv_in;
    int numbytes_in;
    struct sockaddr_storage their_addr_in;
    char buf_in[MAXBUFLEN];
    socklen_t addr_len_in;
    char s_in[INET6_ADDRSTRLEN];

    memset(&hints_in, 0, sizeof hints_in);
    hints_in.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints_in.ai_socktype = SOCK_DGRAM;
    hints_in.ai_flags = AI_PASSIVE; // use my IP


    if ((rv_in = getaddrinfo(LOCAL_IP, LOCAL_PORT, &hints_in, &servinfo_in)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_in));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p_in = servinfo_in; p_in != NULL; p_in = p_in->ai_next) {
        if ((sockfd_in = socket(p_in->ai_family, p_in->ai_socktype,
                p_in->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd_in, p_in->ai_addr, p_in->ai_addrlen) == -1) {
            close(sockfd_in);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p_in == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    //Following taken from Beej talker.c
    int sockfd_out;
    struct addrinfo hints_out, *servinfo_out, *p_out;
    int rv_out;
    int numbytes_out;

    memset(&hints_out, 0, sizeof hints_out);
    hints_out.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints_out.ai_socktype = SOCK_DGRAM;
    hints_out.ai_flags = AI_PASSIVE; // use my IP

     //Listener setup
    if ((rv_out = getaddrinfo(AWS_SERVER_IP, AWS_SERVER_PORT, &hints_out, &servinfo_out)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_out));
        return(1);
    }

    // loop through all the results and bind to the first we can
    for(p_out = servinfo_out; p_out != NULL; p_out = p_out->ai_next) {
        if ((sockfd_out = socket(p_out->ai_family, p_out->ai_socktype,
                p_out->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p_out == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    //Get local port
	struct sockaddr_in local;
	socklen_t l = sizeof(local);
	getsockname(sockfd_in,(struct sockaddr*)&local,(socklen_t *)&l);
    //printf("The ServerA is up and running using UDP on port <%d>\n",(int)ntohs(local.sin_port));
    printf("The ServerA is up and running using UDP on port <%s>\n",LOCAL_PORT);

    while(1){
	    addr_len_in = sizeof their_addr_in;
	    if ((numbytes_in = recvfrom(sockfd_in, buf_in, MAXBUFLEN-1 , 0,
	        (struct sockaddr *)&their_addr_in, &addr_len_in)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }
	    //printf("%s\n",buf_in);

	    char* output;
	    switch(buf_in[0]){
	    	case '1':
	    		output = search(buf_in,myDictionary,'A');
	    		break;
	    	case '2':
	    		output = prefix(buf_in,myPrefixTree,'A');
	    		break;
	    	case '3':
	    		output = suffix(buf_in);
	    		break;
	    	otherwise:
	    		std::cout << "Invalid Request " << buf_in[0]<< std::endl;
	    }

	    if ((numbytes_out = sendto(sockfd_out, output, MAXBUFLEN-1, 0,
	             p_out->ai_addr, p_out->ai_addrlen)) == -1) {
	        perror("talker: sendto");
	        exit(1);
	    }

        std::cout << "The ServerA finished sending the output to AWS" << std::endl;

	}


    close(sockfd_out);

    return 0;
}