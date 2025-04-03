#include "../include/SimpleServer.hpp"

#include "../include/SignalHandler.hpp"

void SimpleServer::launch(void)
{
	while(true && !g_stopFlag)
	{
		if(initPoll() && !g_stopFlag)
		{
			std::cerr << RED << "Poll error, retrying..." << RESET << std::endl;
			continue;  // Skip all after and start with while loop again;
		}
		handlePolls();
	}
}

/************************************************/
/*	Start to Monitor Muliple FileDescriptors	*/
/************************************************/
int SimpleServer::initPoll(void)
{
/*
	poll() returns the number of descriptors that are ready for I/O, or -1 if an error occurred.  If the time limit expires, poll() returns 0.  If poll() returns with
    an error, including one due to an interrupted call, the fds array will be unmodified and the global variable errno will be set to indicate the error.
*/
	int pollCount = poll(_poll_fds.data(), _poll_fds.size(), 1000); // 1 second timeout or -1 to blocking mode
	if (pollCount == 0)
	{
		return 0;
	}	
	else if (pollCount < 0)
	{
		return 1;
	}	
	return 0;
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
				acceptNewConnection(fdIndex);
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


int SimpleServer::isDataToRead(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLIN);
}

bool SimpleServer::isNewConnection(const int& fdIndex)
{
	if(_serverSocket_fds.count(_poll_fds[fdIndex].fd) > 0)
		return true;
	else
		return false;
}


void SimpleServer::acceptNewConnection(const int& fdIndex)
{

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(_poll_fds[fdIndex].fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		std::cerr << RED << "new Client connection FAILED" << strerror(errno) << RESET<< std::endl;
		return;
	}
	std::cout << GREEN << "new Client connection SUCCESSFULL: " << RESET << client_fd << std::endl;
	
	if(fcntl(client_fd, F_SETFL, O_NONBLOCK))
	{
		std::cout << RED << "clientConfiguration FAILED" << RESET << std::endl;	
		return;
	}
	std::cout << GREEN << "clientConfiguration SUCCESSFULL" << RESET << std::endl;
	
	if(client_fd >= 0)
	{
		// make new fd and add it with the read and write flags to the poll_fds container(vector)
		struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
		_poll_fds.push_back(client_poll_fd);
		_recvBuffer[client_fd] = "";
		return;
	}
	else
	{
		std::cout << RED << "UNKNWON CLIENT ERROR" << RESET << std::endl;			
		return;
	}
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


void SimpleServer::removeClient(int fdIndex)
{
	// if keep-alive is requested dont close ?
	int client_fd = _poll_fds[fdIndex].fd;

	std::cout << BG_BRIGHT_RED << "Closing connection: " << RESET << _poll_fds[fdIndex].fd << std::endl;
	
	close(client_fd);
	
	_poll_fds.erase(_poll_fds.begin() + fdIndex);
	_recvBuffer.erase(client_fd);
}

// void SimpleServer::removeClient(int fdIndex)
// {
// 	// if keep-alive is requested dont close ?

// 	std::cout << BG_BRIGHT_RED << "Closing connection: " << RESET << _poll_fds[fdIndex].fd << std::endl;
	
// 	close(_poll_fds[fdIndex].fd);
	
// 	_poll_fds.erase(_poll_fds.begin() + fdIndex);
// 	_recvBuffer.erase(fdIndex);
// }

int	SimpleServer::noDataReceived(int bytesReceived)
{
	// check -1 and 0 seperate !!! eval sheet
	return (bytesReceived <= 0);
}

int SimpleServer::isDataToWrite(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLOUT &&
			(size_t)fdIndex < _recvBuffer.size() &&
			!_recvBuffer[fdIndex].empty());
}

void SimpleServer::handler(int fdIndex)
{
	_request.parseHttpRequest(_recvBuffer[fdIndex]);
	_recvBuffer[fdIndex].clear();

	// std::cout << RED << "requestLine: " << RESET << _request.getRawRequestLine() << std::endl;
	_request.showHeader();
	// _request.showBody();

	_request.handleHttpRequest(_poll_fds[fdIndex].fd);
	removeClient(fdIndex); //???
}
