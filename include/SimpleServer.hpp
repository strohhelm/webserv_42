

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
#include <exception>

#include <fcntl.h>
#include <chrono>

#include <unordered_set>

#include "Macros.hpp"
#include "ServerConfig.hpp"



#ifndef SOCKET_HPP
#define SOCKET_HPP


class SimpleServer
{
	private:
		int										_domain;
		int										_type;
		int										_protocol;
		u_long									_networkInterface;
		struct sockaddr_in						_serviceAddress;
		socklen_t								_serviceAddressLen;
		MainConfig&								_config;

		std::vector<struct pollfd>				_poll_fds;
		
		std::map<int, HttpRequest>				_clients;

		std::map<int, ServerConfig>				_serverConfigs;

		std::unordered_set<int>					_serverSocket_fds;

		// std::unordered_map<int, std::chrono::steady_clock::time_point> _clientLastActivityTimes;
		
		
		public:
		bool									_fatalError = false;
		SimpleServer(int domain, int type, int protocol, u_long networkInterface, MainConfig& config);
		~SimpleServer();
		
		int		serverConfiguration(void);
		int 	createSocket();
		struct sockaddr_in	initAddress(int port);
		int 	bindAddressToSocket(int serverSocket_fd, struct sockaddr_in serviceAddress);
		int		startListenOnSocket(int serverSocket_fd);

		void	setHostnamesToSystem();
		void	removeHostnamesFromSystem();
		std::vector<std::string> readCurrentHosts(std::string pathToFile);
		bool	isHostDefault(std::string host);

		void	launch(void);
		int		initPoll(void);
		void	handlePolls(int pollCount);

		int		isDataToRead(const int&			fdIndex);
		int		isDataToWrite(const int&		fdIndex);
		bool	isNewConnection(const int&		fdIndex);
		void	acceptNewConnection(const int&	fdIndex);
		

		bool	readDataFromClient(int	client_fd);
		void	removeClient(int		client_fd);

		void	closeAllSockets(void);

		void	handler(int fdIndex);

		class ServerConfigException : public std::exception
		{
			public:
				const char* what() const noexcept override;
		};
		int		checkPollError(int fdIndex, bool isServer);

		void	checkIdleConnections(void);


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