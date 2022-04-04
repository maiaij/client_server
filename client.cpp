#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <iostream>

#include "RobustIO.h"

void *chatHandling(void *arg){
    
    int sock = *(int*) arg;

    while(1){
        auto s = RobustIO::read_string(sock);
        std::cout << s << std::endl;
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
	struct addrinfo hints;
	struct addrinfo *addr;
	struct sockaddr_in *addrinfo;
	int rc, rc1, rc2;
	int sock;
	char buffer[512];
	int len;

    //std::string message, user, toServer;
    std::string message, user, toServer;
    bool connected;

    pthread_t tid;
	pthread_attr_t attr;

    pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int *arg = (int *)malloc(sizeof(*arg));

    // Clear the data structure to hold address parameters
    memset(&hints, 0, sizeof(hints));

    // TCP socket, IPv4/IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;

    // Get address info for local host
    rc = getaddrinfo("localhost", NULL, &hints, &addr);
    if (rc != 0) {
        // Note how we grab errors here
        printf("Hostname lookup failed: %s\n", gai_strerror(rc));
        exit(1);
    }

    // Copy the address info, use it to create a socket
    addrinfo = (struct sockaddr_in *) addr->ai_addr;

    sock = socket(addrinfo->sin_family, addr->ai_socktype, addr->ai_protocol);
    if (sock < 0) {
        printf("Can't connect to server\n");
		exit(1);
    }

    // Make sure port is in network order
    addrinfo->sin_port = htons(5555);

    // Connect to the server using the socket and address we resolved
    rc = connect(sock, (struct sockaddr *) addrinfo, addr->ai_addrlen);
    if (rc != 0) {
        printf("Connection failed\n");
        exit(1);
    }

    rc1 = listen(sock, 5);

    // Clear the address struct
    freeaddrinfo(addr);

    std::cout << "Enter your username" << std::endl;
    std::cin >> user;

    std::cout << "Welcome " << user << " !\n" << std::endl;
    auto s = RobustIO::read_string(sock);
    printf("Past Messages:\n%s\n", s.c_str());

    *arg = sock;
    rc2 = pthread_create(&tid, &attr, chatHandling, arg);

    while(rc == 0){
        std::getline(std::cin, message);

        if(message == "exit"){
            RobustIO::write_string(sock, "exit"); 
            rc = 1;
            exit(1);
        }

        else if(!message.empty()){
            toServer = user + ": " + message;
            RobustIO::write_string(sock, toServer); 
            toServer.clear();
        }
    }
    close(sock);

}