#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>
#include <queue>

#include "RobustIO.h"

std::string readHistory(std::queue<std::string> history){
	std::string result = "";

	while(!history.empty()){
		result += history.front();
		result += "\n";

		history.pop();
	}

	return result;
}

int main(int argc, char **argv) {
	int sock, conn;
	int i;
	int rc;
	struct sockaddr address;
	socklen_t addrLength = sizeof(address);
	struct addrinfo hints;
	struct addrinfo *addr;
	char buffer[512];
	int len;

	std::queue<std::string> history;

	// Clear the address hints structure
    memset(&hints, 0, sizeof(hints));

    hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // IPv4/6, socket is for binding
	// Get address info for local host
	if((rc = getaddrinfo(NULL, "5555", &hints, &addr))) {
		printf("host name lookup failed: %s\n", gai_strerror(rc));
		exit(1);
	}

	// Create a socket
    sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if(sock < 0) {
		printf("Can't create socket\n");
		exit(1);
	}

	// Set the socket for address reuse, so it doesn't complain about
	// other servers on the machine.
    i = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	// Bind the socket
    rc = bind(sock, addr->ai_addr, addr->ai_addrlen);
    if(rc < 0) {
		printf("Can't bind socket\n");
		exit(1);
	}

	// Clear up the address data
    freeaddrinfo(addr);

	// Listen for new connections, wait on up to five of them
    rc = listen(sock, 5);
    if(rc < 0) {
		printf("listen failed\n");
		exit(1);
	}

	// When we get a new connection, try reading some data from it!
    while ((conn = accept(sock, (struct sockaddr*) &address, &addrLength)) >= 0) {
        
		
		if(history.empty()){
			RobustIO::write_string(conn, "No Old Messages!!");
		}

		else if(!history.empty()){
			RobustIO::write_string(conn, readHistory(history));
		}

		

		auto s = RobustIO::read_string(sock);
		
		if(!s.empty()){
			std::cout << "BRO PLS MSG IN" << std::endl;
			history.push(s);
				if(history.size() > 12){
					history.pop();
				}
			RobustIO::write_string(conn, readHistory(history));
		}

		
		
		//else if(readHistory(history) != ""){
		//	RobustIO::write_string(conn, readHistory(history));
		//	history.push(RobustIO::read_string(conn));
		//}
		

		

		
		
		//auto s = RobustIO::read_string(conn);
		//printf("Received from client: %s\n", s.c_str());
		
		if(RobustIO::read_string(conn) == "exit"){
			close(conn);
		}
		
    }

}