#include "../include/SimpleServer.hpp"


#include <signal.h>

// fcntl - manipulate file descriptor
// #include <fcntl.h>
// int fcntl(int fd, int op, ... /* arg */ );

/*

F_SETFL (int)
		Set the file status flags to the value specified by arg.
		File access mode (O_RDONLY, O_WRONLY, O_RDWR) and file
		creation flags (i.e., O_CREAT, O_EXCL, O_NOCTTY, O_TRUNC)
		in arg are ignored.  On Linux, this operation can change
		only the O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, and
		O_NONBLOCK flags.  It is not possible to change the O_DSYNC
		and O_SYNC flags; see BUGS, below.

O_NONBLOCK
       By default, both traditional (process-associated) and open file
       description record locks are advisory.  Advisory locks are not
       enforced and are useful only between cooperating processes.

       Both lock types can also be mandatory.  Mandatory locks are
       enforced for all processes.  If a process tries to perform an
       incompatible access (e.g., read(2) or write(2)) on a file region
       that has an incompatible mandatory lock, then the result depends
       upon whether the O_NONBLOCK flag is enabled for its open file
       description.  If the O_NONBLOCK flag is not enabled, then the
       system call is blocked until the lock is removed or converted to a
       mode that is compatible with the access.  If the O_NONBLOCK flag
       is enabled, then the system call fails with the error EAGAIN.

FD_CLOEXEC
		The following operations manipulate the flags associated with a
		file descriptor.  Currently, only one such flag is defined:
		FD_CLOEXEC, the close-on-exec flag.  If the FD_CLOEXEC bit is set,
		the file descriptor will automatically be closed during a
		successful execve(2).  (If the execve(2) fails, the file
		descriptor is left open.)  If the FD_CLOEXEC bit is not set, the
		file descriptor will remain open across an execve(2).
*/
int SimpleServer::serverConfiguration(void)
{
	if(createSocket() < 0)
	{
		perror("Failed to create socket");
		return 1;
	}

	initAddress();

	// Allow reusing the address to avoid "Address already in use" errors
	int opt = 1;
	if (setsockopt(_serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt(SO_REUSEADDR) failed");
		return 1;
	}
	
	// set server to non blocking
	if(fcntl(_serverSocket_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout << RED << "error on serverConfiguration" << RESET << std::endl;	
		return 1;
	}

	// close socket on execve calls
	if(fcntl(_serverSocket_fd, F_SETFD, FD_CLOEXEC) < 0)
	{
		std::cout << RED << "error on serverConfiguration" << RESET << std::endl;	
		return 1;
	}
	if(bindAddressToSocket() < 0)
	{
		// throw execption?
		perror("Failed to bind socket");
		return 1;
	}
	
	if(startListenOnSocket() < 0)
	{
		// throw execption?
		perror("Failed to listen on socket");
		return 1;
	}
	if(_serverSocket_fd >= 0)
	{
		struct pollfd server_poll_fd = {_serverSocket_fd, POLLIN, 0};  // POLLIN means we're interested in reading
		_poll_fds.push_back(server_poll_fd);
		return 0;
	}
	else
	{
		std::cout << "Unknwon error" << std::endl;
		return 1;
	}
}

int SimpleServer::createSocket(void)
{
	_serverSocket_fd = socket(_domain, _type, _protocol);
	return _serverSocket_fd;
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



