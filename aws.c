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

#include "aws_routines.h"

#include <vector>
#include <string>

#define IP_CLIENT "localhost"
#define IP_MONITOR "localhost"
#define IP_BACKEND_A "localhost"
#define IP_BACKEND_B "localhost"
#define IP_BACKEND_C "localhost"
#define IP_LOCAL "localhost"

#define PORT_BACKEND_LOCAL "24951"
#define PORT_CLIENT "25951"
#define PORT_MONITOR "26951"

#define PORT_BACK_REMOTE_A "21951"
#define PORT_BACK_REMOTE_B "22951"
#define PORT_BACK_REMOTE_C "23951"

#define SEARCH "search"
#define PREFIX "prefix"
#define SUFFIX "suffix"

#define BACKLOG 10
#define DATA_SIZE 1000

//Following two functions taken from Beej server.c
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//Get remote port
int getPort(struct sockaddr_storage& remote, char s[INET6_ADDRSTRLEN], int& new_fd){
	inet_ntop(remote.ss_family, get_in_addr((struct sockaddr*)& remote), s, INET6_ADDRSTRLEN);
    struct sockaddr_in addrRemoteMonitor;
    memset(&addrRemoteMonitor, 0, sizeof(addrRemoteMonitor));
    int lenAddrRemoteMonitor = sizeof(addrRemoteMonitor);
    getpeername(new_fd, (struct sockaddr *) &addrRemoteMonitor, (socklen_t *) &lenAddrRemoteMonitor);
    return(addrRemoteMonitor.sin_port);
}

char* getRequestType(char* inBuf){
	char* request;
	if(inBuf[0] == '1'){
		request = (char*)malloc(sizeof(SEARCH));
		strcpy(request,SEARCH);
	}
	else if(inBuf[0] == '2'){
		request = (char*)malloc(sizeof(PREFIX));
		strcpy(request,PREFIX);
	}
	else if(inBuf[0] == '3'){
		request = (char*)malloc(sizeof(SUFFIX));
		strcpy(request,SUFFIX);
	}
	else{
		printf("Invalid request\n");
	}
	return(request);
}

char* getData(char* inClient, const char* port, const char* ip){
	//Following taken from Beej listener.c
	int sockfd_in;
    struct addrinfo hints_in, *servinfo_in, *p_in;
    int rv_in;
    int numbytes_in;
    struct sockaddr_storage their_addr_in;
    char* buf_in = (char*)malloc(DATA_SIZE);
    socklen_t addr_len_in;
    char s_in[INET6_ADDRSTRLEN];

    memset(&hints_in, 0, sizeof hints_in);
    hints_in.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints_in.ai_socktype = SOCK_DGRAM;
    hints_in.ai_flags = AI_PASSIVE; // use my IP

     //Listener setup
    if ((rv_in = getaddrinfo(IP_LOCAL, PORT_BACKEND_LOCAL, &hints_in, &servinfo_in)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_in));
        return(NULL);
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
        return NULL;
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
    if ((rv_out = getaddrinfo(ip, port, &hints_out, &servinfo_out)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_out));
        return(NULL);
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
        return NULL;
    }


    if ((numbytes_out = sendto(sockfd_out, inClient, DATA_SIZE, 0,
             p_out->ai_addr, p_out->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    addr_len_in = sizeof their_addr_in;
    if ((numbytes_in = recvfrom(sockfd_in, buf_in, DATA_SIZE , 0,
        (struct sockaddr *)&their_addr_in, &addr_len_in)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    close(sockfd_in);
    close(sockfd_out);

    return(buf_in);

}


int main(int argc, char const *argv[])
{
	int clientPortRemote;
	int monitorPortRemote;

	//Following taken from Beej server.c
	int sockfd_client, new_fd_client;  
    struct addrinfo hints, *servinfo_client, *p_client;
    struct sockaddr_storage their_addr_client; // connector's address information
    socklen_t sin_size_client;
    int yes_client=1;
    char s_client[INET6_ADDRSTRLEN];
    int rv;

    char buf_client_in[DATA_SIZE];
    char buf_client_out[DATA_SIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(IP_CLIENT, PORT_CLIENT, &hints, &servinfo_client)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p_client = servinfo_client; p_client != NULL; p_client = p_client->ai_next) {
        if ((sockfd_client = socket(p_client->ai_family, p_client->ai_socktype,
                p_client->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd_client, SOL_SOCKET, SO_REUSEADDR, &yes_client,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd_client, p_client->ai_addr, p_client->ai_addrlen) == -1) {
            close(sockfd_client);
            perror("server: bind");
            continue;
     	}

        break;
    }

    freeaddrinfo(servinfo_client); // all done with this structure

    if (p_client == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd_client, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

   	//TCP for monitor

	int sockfd_monitor, new_fd_monitor;  
    struct addrinfo *servinfo_monitor, *p_monitor;
    struct sockaddr_storage their_addr_monitor; // connector's address information
    socklen_t sin_size_monitor;
    int yes_monitor=1;
    char s_monitor[INET6_ADDRSTRLEN];

    char buf_monitor_in[DATA_SIZE];
    char buf_monitor_out[DATA_SIZE];


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(IP_MONITOR, PORT_MONITOR, &hints, &servinfo_monitor)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p_monitor = servinfo_monitor; p_monitor != NULL; p_monitor = p_monitor->ai_next) {
        if ((sockfd_monitor = socket(p_monitor->ai_family, p_monitor->ai_socktype,
                p_monitor->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd_monitor, SOL_SOCKET, SO_REUSEADDR, &yes_monitor,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd_monitor, p_monitor->ai_addr, p_monitor->ai_addrlen) == -1) {
            close(sockfd_monitor);
            perror("server: bind");
            continue;
     	}

        break;
    }

    freeaddrinfo(servinfo_monitor); // all done with this structure

    if (p_monitor == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd_monitor, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

   	//accept monitor connection
   	//accept is blocking so server will not continue until monitor boots up
   	sin_size_monitor = sizeof(their_addr_monitor);
   	new_fd_monitor = accept(sockfd_monitor, (struct sockaddr*)& their_addr_monitor, &sin_size_monitor);
   	if (new_fd_monitor == -1) {
        perror("accept");
    }

    //obtain remote monitor port
    monitorPortRemote = getPort(their_addr_monitor, s_monitor, new_fd_monitor);
    //printf("The remote monitor port is %d\n", monitorPortRemote);

    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }


    printf("The AWS is up and running.\n");
    //start listening for client requests
    while(1){
    	//accept client connections
        sin_size_client = sizeof(their_addr_client);
    	new_fd_client = accept(sockfd_client, (struct sockaddr *)&their_addr_client, &sin_size_client);
        if (new_fd_client == -1) {
            perror("accept");
            exit(1);
        }
            clientPortRemote = getPort(their_addr_client, s_client,new_fd_client);
            if ((recv(new_fd_client, buf_client_in, DATA_SIZE, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            //get function then input then print onto screen
            char* requestType = getRequestType(buf_client_in);
            char* input = buf_client_in + 2;
            printf("The AWS received input=<%s> and function=<%s> from client using TCP over port %d\n",input,requestType,clientPortRemote);
            //printf("%s\n",buf_client_in);
            //perform backend operations
            char* dataA, *dataB, *dataC;
            dataA = getData(buf_client_in,PORT_BACK_REMOTE_A,IP_BACKEND_A);
            printf("The AWS sent input=<%s> and function=<%s> to Backend-ServerA\n",input,requestType);
            dataB = getData(buf_client_in,PORT_BACK_REMOTE_B,IP_BACKEND_B);
            printf("The AWS sent input=<%s> and function=<%s> to Backend-ServerB\n",input,requestType);
            dataC = getData(buf_client_in,PORT_BACK_REMOTE_C,IP_BACKEND_C);
            printf("The AWS sent input=<%s> and function=<%s> to Backend-ServerC\n",input,requestType);

             // printf("dataA: %s\n",dataA);
             // printf("dataB: %s\n",dataB);
             // printf("dataC: %s\n",dataC);

            //Output format (from backend):
            //DIDN'T FIND ANYTHING: 0&INPUT
            //SEARCH AND FOUND SIMILAR: 1&INPUT&VALUE&WORD&DEFINITION
            //SEARCH AND DIDN'T FIND SIMILAR: 1&INPUT&VALUE
            //DIDNT FIND BUT FOUND SIMILAR: 1&&INPUT&WORD&DEFINITION
            //PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn

            //Message format (to client):
            //If not found: '&\0'
            //If search: 'VALUE\0'
            //If prefix or suffix: 'n&VALUE1&VALUE2&VALUE3...&VALUEn\0'

            std::string toClient;
            std::string toMonitor;

            char* toClient_c;
            char* toMonitor_c;

            int countA = 0;
            int countB = 0;
            int countC = 0;
            bool ASimilar = false;
            bool BSimilar = false;
            bool CSimilar = false;
            std::string word1;
            std::string word2;

            switch(buf_client_in[0]){
            	case '1':
                    //client version
                    toClient = aggregateSearchClient(dataA,dataB,dataC);
                    //monitor version
                    toMonitor = aggregateSearchMonitor(dataA,dataB,dataC,word1,word2,ASimilar,BSimilar,CSimilar);
                    if(ASimilar){
                        printf("The AWS received <%d> similar words from Backend-Server A using UDP over port <%s>\n",1,PORT_BACKEND_LOCAL);
                    }
                    else{
                        printf("The AWS received <%d> similar words from Backend-Server A using UDP over port <%s>\n",0,PORT_BACKEND_LOCAL);
                    }
                    if(BSimilar){
                        printf("The AWS received <%d> similar words from Backend-Server B using UDP over port <%s>\n",1,PORT_BACKEND_LOCAL);
                    }
                    else{
                        printf("The AWS received <%d> similar words from Backend-Server B using UDP over port <%s>\n",0,PORT_BACKEND_LOCAL);
                    }
                    if(CSimilar){
                        printf("The AWS received <%d> similar words from Backend-Server C using UDP over port <%s>\n",1,PORT_BACKEND_LOCAL);
                    }
                    else{
                        printf("The AWS received <%d> similar words from Backend-Server C using UDP over port <%s>\n",0,PORT_BACKEND_LOCAL);
                    }
                    
            	    break;
            	case '2':
                    toClient = aggregatePrefixSuffixClient(dataA,dataB,dataC);
                    toMonitor = aggregatePrefixSuffixMonitor(dataA,dataB,dataC,countA, countB, countC);
                    printf("The AWS received <%d> matches from Backend-Server <A> using UDP over port <%s>\n",countA,PORT_BACKEND_LOCAL);
                    printf("The AWS received <%d> matches from Backend-Server <B> using UDP over port <%s>\n",countB,PORT_BACKEND_LOCAL);
                    printf("The AWS received <%d> matches from Backend-Server <C> using UDP over port <%s>\n",countC,PORT_BACKEND_LOCAL);
            	    break;
            	case '3':
                    toClient = aggregatePrefixSuffixClient(dataA,dataB,dataC);
                    toMonitor = aggregatePrefixSuffixMonitor(dataA,dataB,dataC,countA, countB, countC);
                    printf("The AWS received <%d> matches from Backend-Server <A> using UDP over port <%s>\n",countA,PORT_BACKEND_LOCAL);
                    printf("The AWS received <%d> matches from Backend-Server <B> using UDP over port <%s>\n",countB,PORT_BACKEND_LOCAL);
                    printf("The AWS received <%d> matches from Backend-Server <C> using UDP over port <%s>\n",countC,PORT_BACKEND_LOCAL);
                    break;
            }
            toClient_c = new char[toClient.length()+1];
            strcpy(toClient_c,toClient.c_str());

            toMonitor_c = new char[toMonitor.length()+1];
            strcpy(toMonitor_c,toMonitor.c_str());

            //send to client and monitor
            if (send(new_fd_client,toClient_c, toClient.length()+1, 0) == -1){
                perror("send");
            }
            //printf("sent to client:%s\n",toClient_c );

            if (send(new_fd_monitor,toMonitor_c, toMonitor.length()+1, 0) == -1){
                perror("send");
            }
            switch(buf_client_in[0]){
                case '1':
                    printf("The AWS sent <%s> and <%s> to the monitor via TCP port <%d>\n",&word1[0],&word2[0],monitorPortRemote);
                    break;
                case '2':
                    printf("The AWS sent <%d> matches to the monitor via TCP port <%d>\n",countA + countB + countC,monitorPortRemote);  
                    printf("The AWS sent <%d> matches to the client\n",countA + countB + countC); 
                    break;
                case '3':
                    printf("The AWS sent <%d> matches to the monitor via TCP port <%d>\n",countA + countB + countC,monitorPortRemote);
                    printf("The AWS sent <%d> matches to the client\n",countA + countB + countC); 
                    break;
            }
            //printf("sent to monitor:%s\n",toMonitor_c );
            close(new_fd_client);
        


      

    }


	return 0;
}