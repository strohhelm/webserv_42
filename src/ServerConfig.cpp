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