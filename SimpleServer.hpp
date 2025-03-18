

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <poll.h>
#include <fstream>
#include <map>

#include"HttpRequest.hpp"


#ifndef SOCKET_HPP
#define SOCKET_HPP


class SimpleServer
{
	private:
		int					_domain;
		int					_type;
		int					_protocol;
		int					_port;
		u_long				_networkInterface;
		struct sockaddr_in	_address;
		int					_addressLen;
		int					_amountOfConnections;
		
		int					_server_fd;
		int					_bind;
		int					_listen;
		int					_client_fd;

		char 				_buffer[3000];

		struct pollfd		_mypoll;

		// std::map<std::string, std::string> meta;

		HttpRequest			_request;

	public:
	SimpleServer(int domain, int type, int protocol, int port, u_long networkInterface, int amountOfConnections);
		~SimpleServer();
		
		int createSocket(void);
		int bindAddressToSocket(void);
		int startListenOnSocket(void);
		int acceptConnectionsFromSocket(void);

		void	handler(void);
		void	responder(void);
		void	initPoll(void);
		void	launch(void);
		void	parseHTTPRequest(void);
		
		void	connectionTest(int item, std::string message);


		void	setNonBlocking(int fd);
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