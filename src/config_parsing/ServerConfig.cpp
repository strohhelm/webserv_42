#include "../../include/ServerConfig.hpp"

ServerConfig::ServerConfig(std::vector<confToken> context): _port(80), _rootDir("/var/www/html"), _indexFile("index.html"), _errorPage()
{
	_port = 80;
	for (auto it = context.begin(); it != context.end(); it++)
	{

	}
}