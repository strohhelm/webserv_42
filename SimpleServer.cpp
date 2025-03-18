#include "SimpleServer.hpp"


#include <signal.h>

SimpleServer::~SimpleServer()
{
	std::cout << "Destructor called" << std::endl;
	if(_server_fd != -1)
	{
		close(_server_fd);
	}
}

SimpleServer::SimpleServer(int domain, int type, int protocol, int port, u_long networkInterface, int amountOfConnections) :
_domain(domain), _type(type), _protocol(protocol), _port(port), _networkInterface(networkInterface), _amountOfConnections(amountOfConnections)
{
	_server_fd = createSocket();
	connectionTest(_server_fd, "_server_fd");
	
	_bind = bindAddressToSocket();
	connectionTest(_bind, "_bind");
	
	_listen = startListenOnSocket();
	connectionTest(_listen, "_listen");
	
	launch();
}

int SimpleServer::createSocket(void)
{
	return socket(_domain, _type, _protocol);
}

int SimpleServer::startListenOnSocket(void)
{
	return listen(_server_fd, _amountOfConnections);
}


// int SimpleServer::acceptConnectionsFromSocket(void)
// {
// 	initPoll();

// 	std::cout << "Waiting for a connection..." << std::endl;

// 	int pollStatus = poll(&_mypoll, 1, 1000);  // Wait up to 1 second
// 	if (pollStatus < 0)
// 	{
// 		std::cerr << "Poll failed!" << std::endl;
// 		return -1;
// 	}
// 	else if (pollStatus == 0)
// 	{
// 		std::cout << "Poll timeout: No incoming connection." << std::endl;
// 		return -1;
// 	}

// 	_client_fd = accept(_server_fd, (struct sockaddr*)&_address, (socklen_t*)&_addressLen);
// 	connectionTest(_client_fd, "_client_fd");

// 	// Read request from client
// 	int bytesReceived = recv(_client_fd, _buffer, sizeof(_buffer) - 1, MSG_DONTWAIT);
// 	// MSG_DONTWAIT – Perform a non-blocking read.
// 	if (bytesReceived > 0)
// 	{
// 		_buffer[bytesReceived] = '\0'; // Null-terminate received data
// 	}
// 	else
// 	{
// 		std::cerr << "Error receiving data." << std::endl;
// 		return -1;
// 	}

// 	return _client_fd;
// }

#include <fcntl.h> // For fcntl()

void SimpleServer::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl GETFL failed!" << std::endl;
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl SETFL O_NONBLOCK failed!" << std::endl;
    }
}

int SimpleServer::acceptConnectionsFromSocket(void)
{
	initPoll();
	std::cout << "Waiting for a connection..." << std::endl;

	int pollStatus = poll(&_mypoll, 1, 1000);  // Wait up to 1 second
	if (pollStatus < 0)
	{
		std::cerr << "Poll failed!" << std::endl;
		return -1;
	}
	else if (pollStatus == 0)
	{
		std::cout << "Poll timeout: No incoming connection." << std::endl;
		return -1;
	}

	// Check if socket is ready for reading
	if (_mypoll.revents & POLLIN)
	{
		_client_fd = accept(_server_fd, (struct sockaddr*)&_address, (socklen_t*)&_addressLen);
		if (_client_fd < 0)
		{
			std::cerr << "Accept failed!" << std::endl;
			return -1;
		}

		setNonBlocking(_client_fd); // Ensure the client socket is non-blocking

		// Read request from client (only if data is ready)
		int bytesReceived = recv(_client_fd, _buffer, sizeof(_buffer) - 1, 0);
		if (bytesReceived > 0)
		{
			_buffer[bytesReceived] = '\0'; // Null-terminate received data
		}
		else if (bytesReceived == 0)
		{
			std::cerr << "Client closed the connection." << std::endl;
			close(_client_fd);
			return -1;
		}
		else
		{
			std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
			close(_client_fd);
			return -1;
		}
		return _client_fd;
	}
	else
	{
		std::cerr << "No readable data on socket." << std::endl;
		return -1;
	}
}



void SimpleServer::handler(void)
{
	std::cout << "Received request:\n" << _buffer << std::endl;
	_request.parseInput(_buffer);
	if(_request.getPath() == "/exit")
	{
		close(_server_fd);
		close(_client_fd);
	}
}



void SimpleServer::responder(void)
{
	write(_client_fd, _request.getHttpResponse().c_str(), _request.getHttpResponse().size());
	close(_client_fd);	
	// if keep-alive is requested dont close
}



void SimpleServer::initPoll(void)
{
	memset(&_mypoll, 0, sizeof(_mypoll));
	_mypoll.fd = _server_fd;
	_mypoll.events = POLLIN;
}

void SimpleServer::launch(void)
{
	while(true)
	{
		
		_client_fd = acceptConnectionsFromSocket();
		if(_client_fd < 0)
		{
			continue;
		}
		handler();
		responder();
	}
}



int SimpleServer::bindAddressToSocket()
{
	_addressLen = sizeof(_address);
	memset((char*)&_address, 0, _addressLen);
	_address.sin_family = _domain;
	_address.sin_port = htons(_port);
	_address.sin_addr.s_addr = htonl(_networkInterface);

	// for instant restart of server
	// |
	// v
	int opt = 1;
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	return bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address));
}

void SimpleServer::connectionTest(int item, std::string message)
{
	if (item < 0)
	{
		std::cerr << "Failed to connect " << message << std::endl;
		throw std::runtime_error("Socket connection failed.");
	}
	else
	{
		std::cout << "Connection successful" << std::endl;
	}
}