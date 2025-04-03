#include "../include/SimpleServer.hpp"

#include <signal.h>

int SimpleServer::serverConfiguration(void)
{

	for(auto conf : _configs)
	{
		std::cout << "PORT " << conf.getPort() << std::endl;

	}
	for(auto conf : _configs)
	{
		int					serverSocket_fd;
		struct sockaddr_in	serviceAddress;

		serverSocket_fd = createSocket();
		if(serverSocket_fd < 0)
		{
			std::cout << RED << "FAILED TO CREATE SOCKET" << RESET << std::endl;	
			return 1;
		}
		std::cout << GREEN << "SOCKET CREATED" << RESET << std::endl;	
		std::cout << "PORT " << conf.getPort() << std::endl;

		serviceAddress = initAddress(conf.getPort());

		int opt = 1;
		if (setsockopt(serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			std::cout << RED << "SOCKET SET REUSABLE AFTER RESTART FAILED" << RESET << std::endl;	
			return 1;
		}
		std::cout << GREEN << "SOCKET SET REUSABLE AFTER RESTART" << RESET << std::endl;	
		
		if(fcntl(serverSocket_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			std::cout << RED << "SOCKET SET TO NON BLOCKING FAILED" << RESET << std::endl;	
			return 1;
		}
		std::cout << GREEN << "SOCKET SET TO NON BLOCKING SUCCESSFULL" << RESET << std::endl;	
		
		if(fcntl(serverSocket_fd, F_SETFD, FD_CLOEXEC) < 0)
		{
			std::cout << RED << "SOCKET SET TO CLOSE ON EXECVE FAILED" << RESET << std::endl;	
			return 1;
		}
		std::cout << GREEN << "SOCKET SET TO CLOSE ON EXECVE SUCCESSFULL" << RESET << std::endl;	
		
		if(bindAddressToSocket(serverSocket_fd, serviceAddress) < 0)
		{
			std::cout << RED << "BINDING SOCKET TO ADRESS FAILED" << RESET << std::endl;	
			return 1;
		}
		std::cout << GREEN << "BINDING SOCKET TO ADRESS SUCCESSFULL" << RESET << std::endl;	
		
		if(startListenOnSocket(serverSocket_fd) < 0)
		{
			std::cout << RED << "START TO LISTEN ON PORT FAILED" << RESET << std::endl;
			return 1;
		}
		std::cout << GREEN << "START TO LISTEN ON PORT SUCCESSFULL" << RESET << std::endl;

		if(serverSocket_fd >= 0)
		{
			_serverSocket_fds.insert(serverSocket_fd);
			struct pollfd server_poll_fd = {serverSocket_fd, POLLIN, 0};  // POLLIN means we're interested in reading
			_poll_fds.push_back(server_poll_fd);
		}
		else
		{
			std::cout << "UNKNWON SERVER ERROR" << std::endl;
			return 1;
		}
	}
	return 0;
}

int SimpleServer::createSocket(void)
{
	return socket(_domain, _type, _protocol);
	// _serverSocket_fd = socket(_domain, _type, _protocol);
	// return _serverSocket_fd;
}

int SimpleServer::startListenOnSocket(int serverSocket_fd)
{
	return listen(serverSocket_fd, _maxAmountOfConnections);
}

struct sockaddr_in SimpleServer::initAddress(int port)
{
	struct sockaddr_in	serviceAddress;
	socklen_t			serviceAddressLen;

	serviceAddressLen = sizeof(serviceAddress);
	memset((char*)&serviceAddress, 0, serviceAddressLen);
	serviceAddress.sin_family = _domain; //AF_INET
	serviceAddress.sin_port = htons(port);
	serviceAddress.sin_addr.s_addr = htonl(_networkInterface); // INADDR_ANY

	return serviceAddress;
	// _serviceAddressLen = sizeof(_serviceAddress);
	// memset((char*)&_serviceAddress, 0, _serviceAddressLen);
	// _serviceAddress.sin_family = _domain; //AF_INET
	// _serviceAddress.sin_port = htons(_port);
	// _serviceAddress.sin_addr.s_addr = htonl(_networkInterface); // INADDR_ANY
}


int SimpleServer::bindAddressToSocket(int serverSocket_fd, struct sockaddr_in serviceAddress)
{
	return bind(serverSocket_fd, (struct sockaddr*)&serviceAddress, sizeof(serviceAddress));
}


