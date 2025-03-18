#include "SimpleServer.hpp"


#include <signal.h>

SimpleServer::~SimpleServer()
{
	std::cout << "Destructor called" << std::endl;
	for (auto& fd : _poll_fds)
	{
		close(fd.fd);
	}
}

SimpleServer::SimpleServer(int domain, int type, int protocol, int port, u_long networkInterface, int amountOfConnections) :
_domain(domain), _type(type), _protocol(protocol), _port(port), _networkInterface(networkInterface), _amountOfConnections(amountOfConnections)
{
	_serverSocket_fd = createSocket();
	connectionTest(_serverSocket_fd, "_serverSocket_fd");
	
	// Set server to non blocking
	fcntl(_serverSocket_fd, F_SETFL, O_NONBLOCK);

	_bind = bindAddressToSocket();
	connectionTest(_bind, "_bind");
	
	_listenSocket = startListenOnSocket();
	connectionTest(_listenSocket, "_listenSocket");
	
	_server_poll_fd = {_serverSocket_fd, POLLIN, 0};
	_poll_fds.push_back(_server_poll_fd);

	launch();
}

int SimpleServer::createSocket(void)
{
	return socket(_domain, _type, _protocol);
}

int SimpleServer::startListenOnSocket(void)
{
	return listen(_serverSocket_fd, _amountOfConnections);
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

// 	_clientSocket_fd = accept(_serverSocket_fd, (struct sockaddr*)&_clientAddress, (socklen_t*)&_clientAddressLen);
// 	connectionTest(_clientSocket_fd, "_clientSocket_fd");

// 	// Read request from client
// 	int bytesReceived = recv(_clientSocket_fd, _buffer, sizeof(_buffer) - 1, MSG_DONTWAIT);
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

// 	return _clientSocket_fd;
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
	// initPoll();
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
		_clientSocket_fd = accept(_serverSocket_fd, (struct sockaddr*)&_clientAddress, (socklen_t*)&_clientAddressLen);
		if (_clientSocket_fd < 0)
		{
			std::cerr << "Accept failed!" << std::endl;
			return -1;
		}

		setNonBlocking(_clientSocket_fd); // Ensure the client socket is non-blocking

		// Read request from client (only if data is ready)
		int bytesReceived = recv(_clientSocket_fd, _buffer, sizeof(_buffer) - 1, 0);
		if (bytesReceived > 0)
		{
			_buffer[bytesReceived] = '\0'; // Null-terminate received data
		}
		else if (bytesReceived == 0)
		{
			std::cerr << "Client closed the connection." << std::endl;
			close(_clientSocket_fd);
			return -1;
		}
		else
		{
			std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
			close(_clientSocket_fd);
			return -1;
		}
		return _clientSocket_fd;
	}
	else
	{
		std::cerr << "No readable data on socket." << std::endl;
		return -1;
	}
}



void SimpleServer::handler(int index, int fd)
{
	std::cout << "Received request:\n" << _client_buffers[index - 1] << std::endl;
	_request.parseInput(_client_buffers[index - 1], fd);
	// if(_request.getPath() == "/exit")
	// {
	// 	close(_serverSocket_fd);
	// 	close(_clientSocket_fd);
	// }
}



void SimpleServer::responder(void)
{
	write(_clientSocket_fd, _request.getHttpResponse().c_str(), _request.getHttpResponse().size());
	close(_clientSocket_fd);	
	// if keep-alive is requested dont close
}



// void SimpleServer::initPoll(void)
// {
// 	memset(&_mypoll, 0, sizeof(_mypoll));
// 	_mypoll.fd = _serverSocket_fd;
// 	_mypoll.events = POLLIN;
// }


void SimpleServer::acceptNewConnection() {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(_serverSocket_fd, (struct sockaddr*)&client_addr, &client_len);

	if (client_fd < 0) return; // Non-blocking mode: No new connection

	// Make client socket non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	std::cout << "New connection accepted: " << client_fd << std::endl;

	// Add client to poll list
	struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
	_poll_fds.push_back(client_poll_fd);
	_client_buffers.emplace_back();
}

void SimpleServer::handleClient(int index)
{
	int client_fd = _poll_fds[index].fd;
	char buffer[BUFFER_SIZE] = {0};

	int bytesReceived = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytesReceived <= 0)
	{
		removeClient(index);
		return;
	}

	buffer[bytesReceived] = '\0';
	_client_buffers[index - 1] = buffer; // Store request data

	std::cout << "Received: " << buffer << std::endl;
}

void SimpleServer::removeClient(int index) {
    std::cout << "Closing connection: " << _poll_fds[index].fd << std::endl;
    close(_poll_fds[index].fd);
    _poll_fds.erase(_poll_fds.begin() + index);
    _client_buffers.erase(_client_buffers.begin() + (index - 1));
}

void SimpleServer::launch(void)
{
	while(true)
	{
		int pollCount = poll(_poll_fds.data(), _poll_fds.size(), -1);
		if(pollCount < 0)
		{
			perror("Poll failed");
			break;
		}

		for (int i = _poll_fds.size() - 1; i >= 0; i--) // iteration through each file descriptor from backwards. easily to remove 
		{
			if (_poll_fds[i].revents & POLLIN) // data on socket available
			{
				if (_poll_fds[i].fd == _serverSocket_fd) // if event is on server add to poll file descriptors
				{
					acceptNewConnection();
				}
				else
				{
					handleClient(i);
				}
			}

			if (_poll_fds[i].revents & POLLOUT && !_client_buffers[i - 1].empty())
			{
				handler(i, _poll_fds[i].fd);
				// std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
				// send(_poll_fds[i].fd, response.c_str(), response.size(), 0);
				_client_buffers[i - 1].clear(); // Clear after writing
			}
		}
		/*
		_clientSocket_fd = acceptConnectionsFromSocket();
		if(_clientSocket_fd < 0)
		{
			continue;
		}
		handler();
		responder();
		*/
	}
}



int SimpleServer::bindAddressToSocket()
{
	_clientAddressLen = sizeof(_clientAddress);
	memset((char*)&_clientAddress, 0, _clientAddressLen);
	_clientAddress.sin_family = _domain;
	_clientAddress.sin_port = htons(_port);
	_clientAddress.sin_addr.s_addr = htonl(_networkInterface);

	// for instant restart of server
	// |
	// v
	int opt = 1;
	setsockopt(_serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	return bind(_serverSocket_fd, (struct sockaddr*)&_clientAddress, sizeof(_clientAddress));
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