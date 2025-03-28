#include "../include/SimpleServer.hpp"


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
	// try catch?
	if(serverConfiguration())
	{
		close(_serverSocket_fd);
	}
	
	std::cout << BG_BRIGHT_CYAN << "Starting Server" << RESET << std::endl;
	launch();

}



