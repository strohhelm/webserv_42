#include "../include/SimpleServer.hpp"


SimpleServer::~SimpleServer()
{
	std::cout << "Destructor called" << std::endl;
	for (auto& fd : _poll_fds)
	{
		close(fd.fd);
	}
	closeAllSockets();
}

SimpleServer::SimpleServer(int domain, int type, int protocol, u_long networkInterface, int maxAmountOfConnections,std::vector<ServerConfig> configs) :
_domain(domain), _type(type), _protocol(protocol),_networkInterface(networkInterface), _maxAmountOfConnections(maxAmountOfConnections), _configs(configs)
{

	if(serverConfiguration())
	{
		// close(_serverSocket_fd);
		closeAllSockets();
		throw ServerConfigException();
	}
	launch();
}


const char* SimpleServer::ServerConfigException::what() const noexcept
{
	return "ServerConfigError";
}




void SimpleServer::closeAllSockets(void)
{
	for(auto socket : _serverSocket_fds)
	{
		close(socket);
	}
}