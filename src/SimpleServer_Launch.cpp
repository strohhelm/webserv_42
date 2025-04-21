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

/************************************************/
/*	Start to Monitor Muliple FileDescriptors	*/
/************************************************/
int SimpleServer::initPoll(void)
{
/*
	poll() returns the number of descriptors that are ready for I/O, or -1 if an error occurred.  If the time limit expires, poll() returns 0.  If poll() returns with
    an error, including one due to an interrupted call, the fds array will be unmodified and the global variable errno will be set to indicate the error.
*/
	int pollCount = poll(_poll_fds.data(), _poll_fds.size(), 10000); // 1 second timeout or -1 to blocking mode
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
		if (isDataToRead(fdIndex))
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
		if(_done[fdIndex])
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

int SimpleServer::isNewConnection(const int& fdIndex)
{
	return (_poll_fds[fdIndex].fd == _serverSocket_fd);
}

void SimpleServer::acceptNewConnection()
{

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(_serverSocket_fd, (struct sockaddr*)&client_addr, &client_len);
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
	const int buffer_size = 4096;
	char	buffer[buffer_size];
	int		bytesReceived;
	std::string request;
	_done[fdIndex] = false;

	bytesReceived = recv(client_fd, buffer, buffer_size, 0);
	
	if (bytesReceived <= 0)
	{
		std::cerr << "RECV ERROR. " << bytesReceived << std::strerror(errno) << std::endl;
		_recvBuffer[fdIndex].clear();
		_recvHeader[fdIndex].clear();
		_done[fdIndex] = false;
		removeClient(fdIndex);
		return;
	}
	_recvBuffer[fdIndex].append(buffer, bytesReceived);

	std::cout << "buffer size: " <<  _recvBuffer[fdIndex].size() << std::endl;

	if (!_recvHeader[fdIndex].size())
	{
		size_t end = _recvBuffer[fdIndex].find("\r\n\r\n"); // add npos check?
		if (end == std::string::npos)
			return;
		_recvHeader[fdIndex] = _recvBuffer[fdIndex].substr(0, end + 4);
		_headerParsed[fdIndex] = false;
		std::cout << "Header received and stored\n";
		// return;
	}

	if (!_headerParsed[fdIndex])
	{
		size_t begin = _recvHeader[fdIndex].find("Content-Length:");
		if (begin != std::string::npos)
		{
			begin = _recvHeader[fdIndex].find(' ', begin) +1;
			size_t end = _recvHeader[fdIndex].find('\r', begin);
			std::string lenstr = _recvHeader[fdIndex].substr(begin, end - begin);
			if (lenstr.size())
				_clen[fdIndex] = std::stoi(lenstr);

		}
		else
			_clen[fdIndex] = 0;
		_headerParsed[fdIndex] = true;
		std::cout << "Header parsed. clen: " << _clen[fdIndex] << std::endl;
		// return;
	}

	size_t start = _recvBuffer[fdIndex].find("\r\n\r\n") + 4;
	int blen = _recvBuffer[fdIndex].size() - start;

	std::cout << "start: " << start << "size: " << _recvBuffer[fdIndex].size() << std::endl;

	std::cout << "blen: " << blen << "clen: " << _clen[fdIndex] << std::endl;

	if (blen < _clen[fdIndex])
		return;

	std::ofstream rq("request");
		rq << _recvBuffer[fdIndex];
	_done[fdIndex] = true;
	
	return;
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
	return (_poll_fds[fdIndex].revents & POLLOUT && !_recvBuffer[fdIndex].empty());
}

void SimpleServer::handler(int fdIndex)
{
	_request.parseHttpRequest(_recvBuffer[fdIndex]);
	_recvBuffer[fdIndex].clear();
	_recvHeader[fdIndex].clear();
	_done[fdIndex] = false;
	 

	// std::cout << RED << "requestLine: " << RESET << _request.getRawRequestLine() << std::endl;
	_request.showHeader();
	// _request.showBody();

	_request.handleHttpRequest(_poll_fds[fdIndex].fd);
	removeClient(fdIndex); //???
}
