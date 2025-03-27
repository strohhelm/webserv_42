

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <poll.h>
#include <fstream>
#include <unordered_map>

#include <fcntl.h>



#include"HttpRequest.hpp"


#ifndef SOCKET_HPP
#define SOCKET_HPP

#define BUFFER_SIZE 1024

class SimpleServer
{
	private:
		int							_domain;
		int							_type;
		int							_protocol;
		int							_port;
		u_long						_networkInterface;
		struct sockaddr_in			_serviceAddress;
		socklen_t					_serviceAddressLen;
		int							_maxAmountOfConnections;

		int							_serverSocket_fd;
		int							_bind;
		int							_listenSocket;
		int							_clientSocket_fd;

		// char 						_buffer[3000];

		std::vector<struct pollfd>				_poll_fds;
		std::unordered_map<int, std::string>	_recvBuffer;

		HttpRequest			_request;

	public:
		SimpleServer(int domain, int type, int protocol, int port, u_long networkInterface, int maxAmountOfConnections);
		~SimpleServer();
		
		int 	createSocket(void);
		int 	initAddress(void);
		int 	bindAddressToSocket(void);
		int		startListenOnSocket(void);
		void	launch(void);

		int		initPoll(void);
		void	handlePolls(void);


		int		isDataToRead(const int& fdIndex);
		int		isDataToWrite(const int& fdIndex);
		int		isNewConnection(const int& fdIndex);

		void	readDataFromClient(int fdIndex);
		int		noDataReceived(int bytesReceived);
		void	removeClient(int fdIndex);

		void	acceptNewConnection(void);

		// void	shutdown(void);


		// int acceptConnectionsFromSocket(void);

		void	handler(int fdIndex);
		void	responder(void);
		
		void	connectionTest(int item, std::string message);


		// void	setNonBlocking(int fd);
};

#endif



// TCP/IP sockets
// domain:		AF_INET (IP)
// type:		SOCK_STREAM 
// protocol:	0 (zero)

/*
For TCP/IP sockets, we want to specify the IP address family (AF_INET) and virtual circuit service (SOCK_STREAM).
Since there’s only one form of virtual circuit service, there are no variations of the protocol, so the last argument, protocol, is zero
*/


// For IP networking, we use struct sockaddr_in, which is defined in the header netinet/in.h
/*
	struct sockaddr_in 
	{ 
		__uint8_t         sin_len; 
		sa_family_t       sin_family;	// mandatory
		in_port_t         sin_port;		// mandatory
		struct in_addr    sin_addr;		// mandatory
		char              sin_zero[8]; 
	};
*/


/* htonl converts a long integer (e.g. address) to a network representation */ 
/* htons converts a short integer (e.g. port) to a network representation */ 

// _networkInterface -> INADDR_ANY -> 0.0.0.0

// backlog, defines the maximum number of pending connections that can be queued up before connections are refused.