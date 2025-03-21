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

SimpleServer::SimpleServer(int domain, int type, int protocol, int port, u_long networkInterface, int maxAmountOfConnections) :
_domain(domain), _type(type), _protocol(protocol), _port(port), _networkInterface(networkInterface), _maxAmountOfConnections(maxAmountOfConnections)
{
	_serverSocket_fd = createSocket();
	connectionTest(_serverSocket_fd, "_serverSocket_fd");
	// closing if fail
	

	initAddress();

	// for instant restart of server
	// |
	// v
	// int opt = 1;
	// setsockopt(_serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	// // Set server to non blocking
	// fcntl(_serverSocket_fd, F_SETFL, O_NONBLOCK);
	


	_bind = bindAddressToSocket();
	connectionTest(_bind, "_bind");
	// closing if fail
	
	_listenSocket = startListenOnSocket();
	connectionTest(_listenSocket, "_listenSocket");
	// closing if fail?

	
	struct pollfd server_poll_fd = {_serverSocket_fd, POLLIN, 0};  // POLLIN means we're interested in reading
	_poll_fds.push_back(server_poll_fd);

	std::cout << BG_BRIGHT_CYAN << "starting Server" << RESET << std::endl;
	launch();

}

int SimpleServer::createSocket(void)
{
	return socket(_domain, _type, _protocol);
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


// #include <fcntl.h> // For fcntl()

// void SimpleServer::setNonBlocking(int fd)
// {
// 	int flags = fcntl(fd, F_GETFL, 0);
// 	if (flags == -1) {
// 		std::cerr << "fcntl GETFL failed!" << std::endl;
// 		return;
// 	}
// 	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
// 		std::cerr << "fcntl SETFL O_NONBLOCK failed!" << std::endl;
// 	}
// }




void SimpleServer::handler(int index, int fd)
{
	std::cout << "Received request:\n" << _recvBuffer[index - 1] << std::endl;
	_request.parseInput(_recvBuffer[index - 1], fd);
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





void SimpleServer::acceptNewConnection()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(_serverSocket_fd, (struct sockaddr*)&client_addr, &client_len);

	if (client_fd < 0)
	{
		std::cerr << RED << "Error accepting connection: " << strerror(errno) << RESET<< std::endl;
		return;
	}
	
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	std::cout << GREEN << "New connection accepted: " << RESET << client_fd << std::endl;

	// make new fd and add it with the read and write flags to the poll_fds container(vector)
	struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
	_poll_fds.push_back(client_poll_fd);
	// _recvBuffer.emplace(client_fd, "");
	_recvBuffer[client_fd] = "";
}


void SimpleServer::removeClient(int fdIndex)
{
	std::cout << "Closing connection: " << _poll_fds[fdIndex].fd << std::endl;
	
	close(_poll_fds[fdIndex].fd);
	
	_poll_fds.erase(_poll_fds.begin() + fdIndex);
	_recvBuffer.erase(fdIndex);
}

int	SimpleServer::noDataReceived(int bytesReceived)
{
	return (bytesReceived <= 0);
}


void SimpleServer::readDataFromClient(int fdIndex)
{
	int		client_fd = _poll_fds[fdIndex].fd;
	char	buffer[BUFFER_SIZE];
	int		bytesReceived;

	memset(&buffer, '\0', sizeof(buffer));
	
	bytesReceived = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if(noDataReceived(bytesReceived))
	{
		removeClient(fdIndex);
		return ;
	}

	buffer[bytesReceived] = '\0';
	_recvBuffer[fdIndex] = buffer;

	std::cout << BLUE << "Received: " << RESET << buffer << std::endl;
}


int SimpleServer::isDataToRead(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLIN);
}

int SimpleServer::isDataToWrite(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLOUT &&
			fdIndex < _recvBuffer.size() &&
			!_recvBuffer[fdIndex].empty());
}


int SimpleServer::isNewConnection(const int& fdIndex)
{
	return (_poll_fds[fdIndex].fd == _serverSocket_fd);
}



void SimpleServer::handlePolls(void)
{
	int fdIndex = _poll_fds.size() - 1;
	while (fdIndex >= 0)
	{
		if(isDataToRead(fdIndex))
		{
			if(isNewConnection(fdIndex))
			{
				acceptNewConnection();
			}
			else
			{
				readDataFromClient(fdIndex);
				// continue; // Prevent `fdIndex--` since vector size changed
			}
		}
		if(isDataToWrite(fdIndex))
		{
			// handler(fdIndex, _poll_fds[fdIndex].fd);
			_recvBuffer[fdIndex].clear();
		}
		fdIndex--;
	}
}


/************************************************/
/*	Start to Monitor Muliple FileDescriptors	*/
/************************************************/
int SimpleServer::initPoll(void)
{
	int pollCount = poll(_poll_fds.data(), _poll_fds.size(), 1000); // 1 second timeout or -1 to blocking mode
	if (pollCount == 0)
	{
		std::cout << "No events occurred in the timeout period." << std::endl;
	}
	else if (pollCount < 0)
	{
		perror("Poll failed");
		return 1;
	}
	std::cout << "Poll successful, " << pollCount << " events." << std::endl;
	return 0;
}

void SimpleServer::launch(void)
{
	while(true)
	{
		if(initPoll())
		{
			break;
		}
		handlePolls();
	}
}

