#include "../include/SimpleServer.hpp"


#include <signal.h>



void SimpleServer::launch(void)
{
	while(true)
	{
		if(initPoll())
		{
			std::cerr << RED << "Poll error, retrying..." << RESET << std::endl;
			continue;  // Skip all after and start with while loop again;
		}
		handlePolls();
	}
}



void SimpleServer::handler(int fdIndex)
{
	_request.parseHttpRequest(_recvBuffer[fdIndex]);
	_recvBuffer[fdIndex].clear();

	std::cout << RED << "requestLine: " << RESET << _request.getRawRequestLine() << std::endl;
	_request.showHeader();
	_request.showBody();

	_request.handleHttpRequest(_poll_fds[fdIndex].fd);
}




void SimpleServer::acceptNewConnection()
{
	// error handling !?!?!?
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(_serverSocket_fd, (struct sockaddr*)&client_addr, &client_len);

	if (client_fd < 0)
	{
		std::cerr << RED << "Error accepting connection: " << strerror(errno) << RESET<< std::endl;
		return;
	}
	
	if(fcntl(client_fd, F_SETFL, O_NONBLOCK))
		std::cout << RED << "error on clientConfiguration acceptNewConnection" << RESET << std::endl;	


	std::cout << GREEN << "New connection accepted: " << RESET << client_fd << std::endl;

	// make new fd and add it with the read and write flags to the poll_fds container(vector)
	struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
	_poll_fds.push_back(client_poll_fd);
	_recvBuffer[client_fd] = "";
}


void SimpleServer::removeClient(int fdIndex)
{
	// if keep-alive is requested dont close ?

	std::cout << BG_BRIGHT_RED << "Closing connection: " << RESET << _poll_fds[fdIndex].fd << std::endl;
	
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
	std::cout << "readDataFromClient" << std::endl;
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
}


int SimpleServer::isDataToRead(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLIN);
}

int SimpleServer::isDataToWrite(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLOUT &&
			(size_t)fdIndex < _recvBuffer.size() &&
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
		// std::cout << "fdIndex " << fdIndex << std::endl;
		if(isDataToRead(fdIndex))
		{
			if(isNewConnection(fdIndex))
			{
				acceptNewConnection();
			}
			else
			{
				readDataFromClient(fdIndex);
			}
		}
		if(isDataToWrite(fdIndex))
		{
			handler(fdIndex);
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
		// std::cout << "No events occurred in the timeout period" << std::endl;
		return 0;
	}	
	else if (pollCount < 0)
	{
		perror("Poll failed");
		return 1;
	}	
	// std::cout << "Poll successful " << pollCount << " events" << std::endl;
	return 0;
}	

