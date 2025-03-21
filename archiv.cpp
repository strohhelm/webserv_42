
// int SimpleServer::acceptConnectionsFromSocket(void)
// {
// 	// initPoll();
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

// 	// Check if socket is ready for reading
// 	if (_mypoll.revents & POLLIN)
// 	{
// 		_clientSocket_fd = accept(_serverSocket_fd, (struct sockaddr*)&_serviceAddress, (socklen_t*)&_serviceAddressLen);
// 		if (_clientSocket_fd < 0)
// 		{
// 			std::cerr << "Accept failed!" << std::endl;
// 			return -1;
// 		}

// 		setNonBlocking(_clientSocket_fd); // Ensure the client socket is non-blocking

// 		// Read request from client (only if data is ready)
// 		int bytesReceived = recv(_clientSocket_fd, _buffer, sizeof(_buffer) - 1, 0);
// 		if (bytesReceived > 0)
// 		{
// 			_buffer[bytesReceived] = '\0'; // Null-terminate received data
// 		}
// 		else if (bytesReceived == 0)
// 		{
// 			std::cerr << "Client closed the connection." << std::endl;
// 			close(_clientSocket_fd);
// 			return -1;
// 		}
// 		else
// 		{
// 			std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
// 			close(_clientSocket_fd);
// 			return -1;
// 		}
// 		return _clientSocket_fd;
// 	}
// 	else
// 	{
// 		std::cerr << "No readable data on socket." << std::endl;
// 		return -1;
// 	}
// }



// void SimpleServer::initPoll(void)
// {
// 	memset(&_mypoll, 0, sizeof(_mypoll));
// 	_mypoll.fd = _serverSocket_fd;
// 	_mypoll.events = POLLIN;
// }



		/*
		_clientSocket_fd = acceptConnectionsFromSocket();
		if(_clientSocket_fd < 0)
		{
			continue;
		}
		handler();
		responder();
		*/



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

// 	_clientSocket_fd = accept(_serverSocket_fd, (struct sockaddr*)&_serviceAddress, (socklen_t*)&_serviceAddressLen);
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
