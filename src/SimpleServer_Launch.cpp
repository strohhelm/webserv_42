#include "../include/SimpleServer.hpp"
#include "../include/SignalHandler.hpp"



void SimpleServer::checkIdleConnections(void)
{
	auto now = std::chrono::steady_clock::now();
	
	for (auto it = _clientLastActivityTimes.begin(); it != _clientLastActivityTimes.end(); ) {
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
		
		if (elapsed.count() >static_cast<long long> (_config._keepalive_timeout))
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
		// if (debug && pollcount > 0)
		// {
		// 	std::cout<<"Clients: ";
		// 	for (auto &x:_clients)
		// 	{
		// 		(void )x;
		// 		std::cout<< "x ";
		// 	}
		// 	std::cout<<std::endl;
		// }
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
	int client_fd;
	int check = 0;
	while (pollCount && fdIndex >= 0)
	{
		client_fd = _poll_fds[fdIndex].fd;
		// std::cout << "fdIndex " << fdIndex << std::endl;
		if (isDataToRead(fdIndex))
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
		if (_serverSocket_fds.count(client_fd) == 0)
			check = _clients[client_fd].evaluateState();
		if(check == NEEDS_TO_WRITE && isDataToWrite(fdIndex))
		{
			handler(fdIndex);
		}
		else if (check > NEEDS_TO_WRITE  && isDataToWrite(fdIndex))
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"State Error" << RESET<<std::endl;
			_clients[client_fd].sendErrorResponse(check);
		}
		fdIndex--;
	}
}

//


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
	std::cout << GREEN << "new Client connection SUCCESSFULL! " << RESET;
	unsigned char* bytes = (unsigned char*)&client_addr.sin_addr;
	std::cout<<GREEN<< "Client IP: "<<RESET<< (int)bytes[0] << "."<< (int)bytes[1] << "."<< (int)bytes[2] << "."<< (int)bytes[3] <<GREEN<<" fd: "<<RESET<<client_fd<< std::endl;
	if(fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout << RED << "clientConfiguration FAILED" << RESET << std::endl;	
		return;
	}
	std::cout << GREEN << "clientConfiguration SUCCESSFULL" << RESET << std::endl;
	
	// make new fd and add it with the read and write flags to the poll_fds container(vector)
	struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
	_poll_fds.push_back(client_poll_fd);
	_clients[client_fd] = HttpRequest();

	// _recvBuffer[client_fd] = "";

	_clientLastActivityTimes[client_fd] = std::chrono::steady_clock::now();

	_listeningServerFromClient[client_fd] = server_fd;
}


int SimpleServer::readDataFromClient(int fdIndex)
{

	int		client_fd = _poll_fds[fdIndex].fd;
	// const int buffer_size = 4096;
	// char	buffer[buffer_size];
	// int		bytesReceived;
	// std::string request;
	// _done[fdIndex] = false;

	// bytesReceived = recv(client_fd, buffer, buffer_size, 0);
	
	// if (bytesReceived <= 0)
	// {
	// 	std::cerr << "RECV ERROR. " << bytesReceived << std::strerror(errno) << std::endl;
	// 	_recvBuffer[fdIndex].clear();
	// 	_recvHeader[fdIndex].clear();
	// 	_done[fdIndex] = false;
	// 	removeClient(fdIndex);
	// 	return;
	// }
	// _recvBuffer[fdIndex].append(buffer, bytesReceived);

	// std::cout << "buffer size: " <<  _recvBuffer[fdIndex].size() << std::endl;

	// if (!_recvHeader[fdIndex].size())
	// {
	// 	size_t end = _recvBuffer[fdIndex].find("\r\n\r\n"); // add npos check?
	// 	if (end == std::string::npos)
	// 		return;
	// 	_recvHeader[fdIndex] = _recvBuffer[fdIndex].substr(0, end + 4);
	// 	_headerParsed[fdIndex] = false;
	// 	std::cout << "Header received and stored\n";
	// 	// return;
	// }

	// if (!_headerParsed[fdIndex])
	// {
	// 	size_t begin = _recvHeader[fdIndex].find("Content-Length:");
	// 	if (begin != std::string::npos)
	// 	{
	// 		begin = _recvHeader[fdIndex].find(' ', begin) +1;
	// 		size_t end = _recvHeader[fdIndex].find('\r', begin);
	// 		std::string lenstr = _recvHeader[fdIndex].substr(begin, end - begin);
	// 		if (lenstr.size())
	// 			_clen[fdIndex] = std::stoi(lenstr);

	// 	}
	// 	else
	// 		_clen[fdIndex] = 0;
	// 	_headerParsed[fdIndex] = true;
	// 	std::cout << "Header parsed. clen: " << _clen[fdIndex] << std::endl;
	// 	// return;
	// }

	// size_t start = _recvBuffer[fdIndex].find("\r\n\r\n") + 4;
	// int blen = _recvBuffer[fdIndex].size() - start;

	// std::cout << "start: " << start << "size: " << _recvBuffer[fdIndex].size() << std::endl;

	// std::cout << "blen: " << blen << "clen: " << _clen[fdIndex] << std::endl;

	// if (blen < _clen[fdIndex])
	// 	return;

	// std::ofstream rq("request");
	// 	rq << _recvBuffer[fdIndex];
	// _done[fdIndex] = true;
	
	// return;
	HttpRequest& client = _clients[client_fd]; // reference to current client
	if (debug)std::cout << ORANGE<<"ReadDataFromClient" <<RESET<< std::endl;


	char	buffer[BUFFER_SIZE];
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
	// if (debug) std::cout << std::string(buffer, bytesReceived) << std::endl;
	if (bytesReceived == 0)
	{
		// Connection was closed by the client
		std::cout << "CLIENT " << client_fd << " closed the connection." << std::endl;
		removeClient(fdIndex);
		return 0;
	}
	if (client._state._isNewRequest)
		client._state._isNewRequest = false;
	client._state._buffer.append(std::string(buffer, bytesReceived));
	_clientLastActivityTimes[client_fd] = std::chrono::steady_clock::now();
	return 1;
}


void SimpleServer::removeClient(int fdIndex)
{
	int client_fd = _poll_fds[fdIndex].fd;
	// close(client_fd);
	// _poll_fds.erase(_poll_fds.begin() + fdIndex);
	// Close the client socket
	if (debug)std::cout<<ORANGE<<"removing client on fd: "<<client_fd<<RESET<<std::endl;
	close(client_fd);
	for (auto it = _poll_fds.begin(); it != _poll_fds.end(); ++it)
	{
		if (it->fd == client_fd)
		{
			_poll_fds.erase(it);
			break;  // Exit after removing the client
		}
	}
	_clients.erase(client_fd);
}


// int	SimpleServer::noDataReceived(int bytesReceived)
// {
// 	// check -1 and 0 seperate !!! eval sheet
// 	return (bytesReceived <= 0);
// }

// int SimpleServer::isDataToWrite(const int& fdIndex)
// {
// 	return (_poll_fds[fdIndex].revents & POLLOUT &&
// 			(size_t)fdIndex < _recvBuffer.size() &&
// 			!_recvBuffer[fdIndex].empty());
// }

int SimpleServer::isDataToWrite(const int& fdIndex)
{
    return (_poll_fds[fdIndex].revents & POLLOUT);
}



void SimpleServer::handler(int fdIndex)
{
	// _request.parseHttpRequest(_recvBuffer[fdIndex]);
	// _recvBuffer[fdIndex].clear();
	
	// std::cout << RED << "requestLine: " << RESET << _request.getRawRequestLine() << std::endl;
	// _request.showHeader();
	// _request.showBody();
	
	int client_fd = _poll_fds[fdIndex].fd;
	int server_fd = _listeningServerFromClient[client_fd];
	// ServerConfig& config = _serverConfigs[server_fd];
	HttpRequest &client = _clients[client_fd];
	if (client._state._isValidRequest == 0)
	{
		client._state._isNewRequest = false;
		client._config = &(_serverConfigs[server_fd]);
		client._client_fd = client_fd;
		client._server_fd = server_fd;
		int invalid = client.validateRequest();
		if (invalid != 0)
		{
			client._state._isValidRequest = -1;
			int code = 404;
			if (invalid < 0)
				code = 400;
			if (debug)std::cout << BG_BRIGHT_RED<<"invalid request " << RESET<<std::endl;
			client.sendErrorResponse(code);
			return;
		}
		client._state._isValidRequest = 1;
	}
	client.handleHttpRequest();
}
