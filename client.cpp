#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

#include "RobustIO.h"

int main(int argc, char **argv) {
	struct addrinfo hints;
	struct addrinfo *addr;
	struct sockaddr_in *addrinfo;
	int rc;
	int sock;
	char buffer[512];
	int len;

    std::string message, user, toServer;

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

    // Clear the address struct
    freeaddrinfo(addr);

    // Write a test string to the server.
    //RobustIO::write_string(sock, "testing the new functions!!");
    //auto s = RobustIO::read_string(sock);
    //printf("Received from SERVER: %s\n", s.c_str());

    std::cout << "Enter your username" << std::endl;
    std::cin >> user;

    toServer += user;
    toServer += ": ";

    std::cout << "Welcome " << user << " !" << std::endl;

    while(message != "exit"){
        auto s = RobustIO::read_string(sock);
        printf("Past Messages:\n%s\n", s.c_str());
        
        std::cout << user << ": ";
        //std::getline(std::cin, message);
        std::cin >> message;

        //toServer = user + ": " + message;
        toServer += message;

        //std::cout << "MESSAGE: " << message << std::endl;

        if(!message.empty()){
            std::cout << "writing to server" << std::endl;
            RobustIO::write_string(sock, toServer); 
            toServer = "";
        }
        
        
    }

    RobustIO::write_string(sock, "exit"); 

    //RobustIO::

	close(sock);

    

}