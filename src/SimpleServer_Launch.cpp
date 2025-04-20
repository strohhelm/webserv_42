#include "../include/SimpleServer.hpp"

#include "../include/SignalHandler.hpp"



void SimpleServer::checkIdleConnections(void)
{
	auto now = std::chrono::steady_clock::now();
	
	for (auto it = _clientLastActivityTimes.begin(); it != _clientLastActivityTimes.end(); ) {
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
		
		if (elapsed.count() > 10)
		{
			std::cout << "Closing idle connection on fd: " << it->first << std::endl;
			close(it->first);
			it = _clientLastActivityTimes.erase(it);
		}
		else
		{
			++it;
		}
	}
}


void SimpleServer::launch(void)
{
	while(!g_stopFlag)
	{
		int pollcount = initPoll();
		if(pollcount  < 0 && !g_stopFlag)
		{
			std::cerr << RED << "Poll error, retrying..." << RESET << std::endl;
			continue; 
		}
		handlePolls(pollcount);
		checkIdleConnections();
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
	int pollCount = poll(_poll_fds.data(), _poll_fds.size(), 200);
	// if (pollCount == 0)
	// {
	// 	// std::cout << "pollCount = 0" << std::endl;
	// 	return 0;
	// }	
	// else if (pollCount < 0)
	// {
	// 	//std::cout << "pollCount = <1" << std::endl;
	// 	return pollCount;
	// }	
	//std::cout << "else poll" << std::endl;
	return pollCount;
}	


void SimpleServer::handlePolls(int pollCount)
{
	int fdIndex = _poll_fds.size() - 1;
	while (pollCount && fdIndex >= 0)
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
	int server_fd = _poll_fds[fdIndex].fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		std::cerr << RED << "new Client connection FAILED" << strerror(errno) << RESET<< std::endl; //ERNO !?!?!?!?!?!?!??!
		return;
	}
	std::cout << GREEN << "new Client connection SUCCESSFULL: " << RESET << client_fd << std::endl;
	
	if(fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout << RED << "clientConfiguration FAILED" << RESET << std::endl;	
		return;
	}
	std::cout << GREEN << "clientConfiguration SUCCESSFULL" << RESET << std::endl;
	
	// make new fd and add it with the read and write flags to the poll_fds container(vector)
	struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
	_poll_fds.push_back(client_poll_fd);
	_recvBuffer[client_fd] = "";

	_clientLastActivityTimes[client_fd] = std::chrono::steady_clock::now();

	_listeningServerFromClient[client_fd] = server_fd;
}


int SimpleServer::readDataFromClient(int fdIndex)
{
	std::cout << "readDataFromClient" << std::endl;
	int		client_fd = _poll_fds[fdIndex].fd;

	static char	buffer[BUFFER_SIZE];
	int		bytesReceived;
	memset(&buffer, '\0', sizeof(buffer));
	
	bytesReceived = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytesReceived < 0)
	{
		// Handle error case (recv() failed)
		std::cerr << RED << "Error receiving data from client: " << strerror(errno) << RESET << std::endl; //ERNO!?!?!?!??!?
		removeClient(fdIndex);  // Remove client on error
		return 0;
	}

	// std::cout << std::string(buffer, bytesReceived) << std::endl;
	if (bytesReceived == 0)
	{
		// Connection was closed by the client
		std::cout << "CLIENT " << client_fd << " closed the connection." << std::endl;
		removeClient(fdIndex);
		return 0;
	}
	
	_recvBuffer[fdIndex].append(std::string(buffer, bytesReceived));
	_clientLastActivityTimes[client_fd] = std::chrono::steady_clock::now();

	return 1;
}


void SimpleServer::removeClient(int fdIndex)
{
	int client_fd = _poll_fds[fdIndex].fd;
	// close(client_fd);
	// _poll_fds.erase(_poll_fds.begin() + fdIndex);
	// Close the client socket
	close(client_fd);
	for (auto it = _poll_fds.begin(); it != _poll_fds.end(); ++it)
	{
		if (it->fd == client_fd)
		{
			_poll_fds.erase(it);
			break;  // Exit after removing the client
		}
	}
	_recvBuffer.erase(fdIndex);
}


int	SimpleServer::noDataReceived(int bytesReceived)
{
	// check -1 and 0 seperate !!! eval sheet
	return (bytesReceived <= 0);
}

// int SimpleServer::isDataToWrite(const int& fdIndex)
// {
// 	return (_poll_fds[fdIndex].revents & POLLOUT &&
// 			(size_t)fdIndex < _recvBuffer.size() &&
// 			!_recvBuffer[fdIndex].empty());
// }

int SimpleServer::isDataToWrite(const int& fdIndex)
{
    return (_poll_fds[fdIndex].revents & POLLOUT && !_recvBuffer[fdIndex].empty());
}



void SimpleServer::handler(int fdIndex)
{
	_request.parseHttpRequest(_recvBuffer[fdIndex]);
	_recvBuffer[fdIndex].clear();
	
	std::cout << RED << "requestLine: " << RESET << _request.getRawRequestLine() << std::endl;
	_request.showHeader();
	_request.showBody();
	
	int client_fd = _poll_fds[fdIndex].fd;
	int server_fd = _listeningServerFromClient[client_fd];
	routeConfig route;
	int invalid =  _request.validateRequest(_serverConfigs[server_fd], route);
	if (invalid != 0)
	{
		int code = 404;
		if (invalid < 0)
			code = 400;
		_request.sendErrorResponse(client_fd, code, _serverConfigs[server_fd]);
		return;
	}
	_request.handleHttpRequest(client_fd, server_fd, _serverConfigs[server_fd], route);

	// removeClient(fdIndex);
}
