#include "../include/SimpleServer.hpp"
#include "../include/SignalHandler.hpp"



void SimpleServer::checkIdleConnections(void)
{
	auto now = std::chrono::steady_clock::now();
	std::vector<int> clients_to_remove;
	clients_to_remove.clear();
	for (auto it = _clients.rbegin(); it != _clients.rend(); it++)
	 {
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second._state._lastActivity);
		if (elapsed.count() >static_cast<long long> (_config._keepalive_timeout))
		{
			clients_to_remove.push_back(it->first);
		}
	}
	for (int fd : clients_to_remove)
	{
		std::cout <<YELLOW<< "Closing connection on fd: " << MAGENTA<<fd<< YELLOW<<" -> connection timeout"<<RESET<< std::endl;
		removeClient(fd);
	}
}


void SimpleServer::launch(void)
{
	while(!g_stopFlag)
	{
		int pollcount = initPoll();
		if(pollcount  < 0 && !g_stopFlag)
		{
			std::cerr << BG_BRIGHT_RED << "Poll error, retrying..." << RESET << std::endl;
			continue; 
		}
		handlePolls(pollcount);
		if (_fatalError)
			break ;
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
	return  poll(_poll_fds.data(), _poll_fds.size(), 200);
}

int	SimpleServer::checkPollError(int fdIndex, bool isServer)
{
	try{
		auto& current = _poll_fds.at(fdIndex);
		if (!isServer)
		{
			HttpRequest &client = _clients.at(current.fd);
			if (current.revents &( POLLHUP | POLLERR))
				{removeClient(client._client_fd); return 1;}
			else if (current.revents & POLLNVAL)
			{
				std::cout<<BG_BRIGHT_RED<<"CLIENT ERROR:"<<RESET<<RED<<" POLLNVAL on clientfd "<<MAGENTA<<client._client_fd<<RESET<<std::endl;
				_poll_fds.erase(_poll_fds.begin() + fdIndex);
				if(_clients.count(current.fd))
				{
					_clients.erase(current.fd);
					return 1;
				}
			}
		}
		else if (isServer)
		{
			if (current.revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				std::cout<<BG_BRIGHT_RED<<"FATAL ERROR:"<<RESET<<RED<<"serverfd "<<MAGENTA<<_poll_fds[fdIndex].fd<<RESET<<std::endl;
				return 2;
			}
		}
		return 0;
	}catch(std::exception& e){
		std::cout<<RED<<"lol check error poll fail! "<<e.what()<<RESET<<std::endl;
		return -1;
	}
}


void SimpleServer::handlePolls(int pollCount)
{
	
	int client_fd;
	int check = 0;
	for (int fdIndex = _poll_fds.size() - 1; pollCount > 0 && fdIndex >= 0; fdIndex--)
	{
		if (_poll_fds[fdIndex].revents == 0)
			continue;
		pollCount--;
		client_fd = _poll_fds[fdIndex].fd;
		bool isServer = (_serverSocket_fds.count(client_fd));
		int x = checkPollError(fdIndex, isServer);
		if (x == 2)
		{
			_fatalError = true;
			return ;
		}
		else if (x != 0)
			continue;
		if(isServer && isDataToRead(fdIndex))
			acceptNewConnection(fdIndex);
		else
		{
			if (isDataToRead(fdIndex))
				readDataFromClient(client_fd);
			try {
				HttpRequest &client = _clients.at(client_fd);
				check = client.evaluateState();
				if(check == NEEDS_TO_WRITE && isDataToWrite(fdIndex))
				{
					handler(fdIndex);
				}
				else if (check > NEEDS_TO_WRITE  && isDataToWrite(fdIndex))
				{
					if (debug)std::cout << BG_BRIGHT_RED<<"State Error" << RESET<<std::endl;
					client.sendErrorResponse(check);
				}
				if (std::find(CloseCodes.begin(), CloseCodes.end(),client._state._errorOcurred) != CloseCodes.end())
				{
					std::cout<<BG_BRIGHT_RED<<"Remove because: "<<client._state._errorOcurred<<RESET<<std::endl;
					removeClient(client_fd);
				}
				else
					client._state._errorOcurred = 0;
			}catch(...){}
		}
		
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
		std::cerr << RED << "new Client connection FAILED" << RESET<< std::endl;
		return;
	}
	std::cout << BG_BRIGHT_GREEN << "new Client connection SUCCESSFULL!" << RESET;
	unsigned char* bytes = (unsigned char*)&client_addr.sin_addr;
	std::cout<<GREEN<< " Client IP: "<<YELLOW<< (int)bytes[0] << "."<< (int)bytes[1] << "."<< (int)bytes[2] << "."<< (int)bytes[3] <<GREEN<<" fd: "<<YELLOW<<client_fd<< RESET<<std::endl;
	if(fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout << RED << "clientConfiguration FAILED" << RESET << std::endl;	
		return;
	}
	std::cout << GREEN << "Client configuration SUCCESSFULL" << RESET << std::endl;
	
	// make new fd and add it with the read and write flags to the poll_fds container(vector)
	struct pollfd client_poll_fd = {client_fd, POLLIN | POLLOUT, 0};
	_poll_fds.push_back(client_poll_fd);
	_clients.insert({client_fd, HttpRequest(client_fd, &(_serverConfigs[server_fd]))});
}


bool SimpleServer::readDataFromClient(int client_fd)
{
	try{
	HttpRequest& client = _clients.at(client_fd); //reason for try{} // reference to current client
	if (debug)std::cout << ORANGE<<"ReadDataFromClient" <<RESET<< std::endl;


	char	buffer[BUFFER_SIZE];
	int		bytesReceived;
	memset(&buffer, '\0', sizeof(buffer));
	
	bytesReceived = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytesReceived < 0)
	{
		// Handle error case (recv() failed)
		std::cerr << RED << "Error receiving data from client: " << RESET << std::endl; 
		removeClient(client_fd); 
		return 0;
	}
	if (bytesReceived == 0)
	{
		// Connection was closed by the client
		std::cout << YELLOW << "CLIENT " << MAGENTA<< client_fd <<YELLOW<< " closed the connection."<<RESET << std::endl;
		removeClient(client_fd);
		return 0;
	}
	if (client._state._isNewRequest)
		client._state._isNewRequest = false;
	client._state._buffer.append(std::string(buffer, bytesReceived));
	client._state._lastActivity = std::chrono::steady_clock::now();
	return 1;
	}
	catch(std::exception &e ){std::cout<<BG_BRIGHT_RED<<"ERROR IN READ DATA FUNCTION!: "<<e.what()<<RESET<<std::endl;
	return 0;}
}


void SimpleServer::removeClient(int client_fd)
{
	if (debug)std::cout<<ORANGE<<"removing client on fd: "<<client_fd<<RESET<<std::endl;
	close(client_fd);
	for (auto it = _poll_fds.begin(); it != _poll_fds.end(); ++it)
	{
		if (it->fd == client_fd)
		{
			_poll_fds.erase(it);
			break;
		}
	}
	_clients.erase(client_fd);
}


int SimpleServer::isDataToWrite(const int& fdIndex)
{
	return (_poll_fds[fdIndex].revents & POLLOUT);
}



void SimpleServer::handler(int fdIndex)
{
	try{
	int client_fd = _poll_fds.at(fdIndex).fd;
	HttpRequest &client = _clients.at(client_fd);
	if (client._state._isValidRequest == 0)
	{
		client._state._isNewRequest = false;
		int invalid = client.validateRequest();
		if (invalid != 0)
		{
			client._state._isValidRequest = -1;
			if (debug)std::cout << BG_BRIGHT_RED<<"invalid request " << RESET<<std::endl;
			client.sendErrorResponse(invalid);
			return;
		}
		if (debug)std::cout << ORANGE << "Request Valid" << RESET << std::endl;
		if (debug)std::cout<<ORANGE<<"Route: "<<RESET << (*(client._route))._path <<std::endl;
		client._state._isValidRequest = 1;
	}
	client.handleHttpRequest();
}catch(...){}
}
