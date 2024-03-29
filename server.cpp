#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>
#include <iostream>
#include <queue>
#include <list>

#include "RobustIO.h"

std::queue<std::string> history;
std::list<int> users;

std::string readHistory(std::queue<std::string> history){
	std::string result = "";

	while(!history.empty()){
		result += history.front();
		result += "\n";

		history.pop();
	}

	return result;
}

void *chatHandling(void *conn){
	bool connected = true;
	std::string msg;

	if(history.empty()){
		RobustIO::write_string(*((int *)conn), "No Old Messages!!\n");
	}

	else{
		RobustIO::write_string(*((int *)conn), readHistory(history));
	}

	// When we get a new connection, try reading some data from it!
    while (connected) {

		auto s = RobustIO::read_string(*((int *)conn));
		msg = s.c_str();

		if(msg == "exit"){
			close(*((int *)conn));
		}

		history.push(msg);

		if(history.size() > 12){
			history.pop();
		}

		for(int i: users){
			//if(*((int *)conn) != i){
				RobustIO::write_string(i, history.back());
			//}
		}
		
    }

	pthread_exit(NULL);
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
	
	

	//variables for the multithreading
	int clientMax = 3;
	int clients[clientMax];
	int counter = 0;
	pthread_t tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int *arg = (int *)malloc(sizeof(*arg));

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

	// mutithread
	while(counter != clientMax){
		clients[counter] = accept(sock, (struct sockaddr*) &address, &addrLength);
		
		if(clients[counter] < 0){
			printf("accepting client failed\n");
			exit(1);
		}

		users.push_back(clients[counter]);

		*arg = clients[counter];
		rc = pthread_create(&tid, &attr, chatHandling, arg);
		counter++;

		
	}

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);

	for (int i = 0; i < clientMax; i++){
		close(clients[i]);
	}

	close(sock);
	

}