#include"../include/ServerConfig.hpp"


ServerConfig::ServerConfig()
{
	
}

ServerConfig::~ServerConfig()
{
	
}


void ServerConfig::setUrl(const std::vector<std::string>& serverNames ,const int& port)
{
	_serverNames = serverNames;
	_port = port;
}


int		ServerConfig::getPort(void)
{
	return _port;
}


void	ServerConfig::setRootDir(const std::string& rootDir)
{
	_rootDir = rootDir;
}

const std::string& ServerConfig::getRootDir(void)
{
	return _rootDir;
}