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

void	ServerConfig::setCgiPath(std::string path)
{
	_cgiPath = path;
}

std::string& ServerConfig::getCgiPath(void)
{
	return _cgiPath;
}


bool	ServerConfig::isDirListingActive(std::string location)
{
	for(auto it : _locations)
	{
		if(it.first == location)
		{
			if(it.second._dirListing == true)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}