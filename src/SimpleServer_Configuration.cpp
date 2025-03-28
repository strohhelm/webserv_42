#include "../include/SimpleServer.hpp"

#include <signal.h>

int SimpleServer::serverConfiguration(void)
{
	if(createSocket() < 0)
	{
		std::cout << RED << "FAILED TO CREATE SOCKET" << RESET << std::endl;	
		return 1;
	}
	std::cout << GREEN << "SOCKET CREATED" << RESET << std::endl;	

	initAddress();

	int opt = 1;
	if (setsockopt(_serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cout << RED << "SOCKET SET REUSABLE AFTER RESTART FAILED" << RESET << std::endl;	
		return 1;
	}
	std::cout << GREEN << "SOCKET SET REUSABLE AFTER RESTART" << RESET << std::endl;	
	
	if(fcntl(_serverSocket_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout << RED << "SOCKET SET TO NON BLOCKING FAILED" << RESET << std::endl;	
		return 1;
	}
	std::cout << GREEN << "SOCKET SET TO NON BLOCKING" << RESET << std::endl;	
	
	if(fcntl(_serverSocket_fd, F_SETFD, FD_CLOEXEC) < 0)
	{
		std::cout << RED << "SOCKET SET TO CLOSE ON EXECVE FAILED" << RESET << std::endl;	
		return 1;
	}
	std::cout << GREEN << "SOCKET SET TO CLOSE ON EXECVE" << RESET << std::endl;	
	
	if(bindAddressToSocket() < 0)
	{
		std::cout << RED << "BINDING SOCKET TO ADRESS FAILED" << RESET << std::endl;	
		return 1;
	}
	std::cout << GREEN << "BINDING SOCKET TO ADRESS SUCCESSFULL" << RESET << std::endl;	
	
	if(startListenOnSocket() < 0)
	{
		std::cout << RED << "START TO LISTEN ON PORT FAILED" << RESET << std::endl;
		return 1;
	}
	std::cout << GREEN << "START TO LISTEN ON PORT SUCCESSFULL" << RESET << std::endl;

	if(_serverSocket_fd >= 0)
	{
		struct pollfd server_poll_fd = {_serverSocket_fd, POLLIN, 0};  // POLLIN means we're interested in reading
		_poll_fds.push_back(server_poll_fd);
		return 0;
	}
	else
	{
		std::cout << "Unknwon error" << std::endl;
		return 1;
	}
}

int SimpleServer::createSocket(void)
{
	_serverSocket_fd = socket(_domain, _type, _protocol);
	return _serverSocket_fd;
}

int SimpleServer::startListenOnSocket(void)
{
	return listen(_serverSocket_fd, _maxAmountOfConnections);
}

int SimpleServer::initAddress(void)
{
	_serviceAddressLen = sizeof(_serviceAddress);
	memset((char*)&_serviceAddress, 0, _serviceAddressLen);
	_serviceAddress.sin_family = _domain;
	_serviceAddress.sin_port = htons(_port);
	_serviceAddress.sin_addr.s_addr = htonl(_networkInterface);
	return 0;
}


int SimpleServer::bindAddressToSocket()
{
	return bind(_serverSocket_fd, (struct sockaddr*)&_serviceAddress, sizeof(_serviceAddress));
}


