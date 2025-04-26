#include "../include/SimpleServer.hpp"


SimpleServer::~SimpleServer()
{
	std::cout << "Destructor called" << std::endl;
	for (auto& fd : _poll_fds)
	{
		close(fd.fd);
	}
	closeAllSockets();
	removeHostnamesFromSystem();
}

SimpleServer::SimpleServer(int domain, int type, int protocol, u_long networkInterface, MainConfig& config) :
_domain(domain), _type(type), _protocol(protocol),_networkInterface(networkInterface), _config(config)
{

	if(serverConfiguration())
	{
		// close(_serverSocket_fd);
		closeAllSockets();
		throw ServerConfigException();
	}
	
	setHostnamesToSystem();
	launch();
}

std::vector<std::string> SimpleServer::readCurrentHosts(std::string pathToFile)
{
	// std::cout << "readCurrentHosts\n";
	std::vector<std::string> hostsInFile;
	std::string buffer;
	std::fstream readFile;

	readFile.open(pathToFile, std::ios::in);
	if(readFile.fail() == true || !readFile.is_open())
	{
		return {};
	}
	if(readFile.is_open())
	{
		while(getline(readFile, buffer))
		{
			hostsInFile.push_back(buffer);
		}
		readFile.close();
	}
	return hostsInFile;
}

bool SimpleServer::isHostDefault(std::string host)
{
	for(auto& conf : _config._http)
	{
		for (auto& name : conf._serverNames)
		{
			if(host.find(name) != std::string::npos)
			{
				return false;
			}
		}
	}
	return true;
}

void SimpleServer::removeHostnamesFromSystem()
{
	const std::string pathToFile = "/etc/hosts";
	std::vector<std::string> hostsInFile = readCurrentHosts(pathToFile);

	if (hostsInFile.empty())
	{
		std::cerr << "Error: could not read " << pathToFile << "\n";
		return;
	}

	std::ofstream writeFile(pathToFile);
	if (writeFile.fail() || !writeFile.is_open())
	{
		std::cerr << "Error: could not open " << pathToFile << "\n";
		return;
	}

	for (const auto& host : hostsInFile)
	{
		if (isHostDefault(host) || host.find("localhost") != std::string::npos)
		{
			// std::cout << "Added: " << host << "\n";
			writeFile << host << "\n";
		}
		else
		{
			// std::cout << "Removed: " << host << "\n";
		}
	}
}

void SimpleServer::setHostnamesToSystem(void)
{
	const std::string pathToFile = "/etc/hosts";
	std::fstream writeFile;

	writeFile.open(pathToFile, std::ios::app); //app -> append
	if(writeFile.fail() == true)
	{
		// error
	}
	if(writeFile.is_open())
	{
		for(auto& conf : _config._http )
		{
			for (auto& name : conf._serverNames)
			{
				writeFile << "127.0.0.1 " << name << "\n";
			}
		}
		writeFile.close();
	}
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