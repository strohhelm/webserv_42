#include "../../include/ServerConfig.hpp"

void	ServerConfig::parseTokens(std::vector<confToken>	&tokens, std::map <std::string, void(ServerConfig::*)(std::vector<confToken> &)> directives)
{

}

void	ServerConfig::setPort(std::vector<confToken>			&context)
{
	
}
void	ServerConfig::setServerNames(std::vector<confToken>		&context)
{
	
}
void	ServerConfig::setErrorPages(std::vector<confToken>		&context)
{
	
}
void	ServerConfig::setClientBodySize(std::vector<confToken>	&context)
{
	
}
void	ServerConfig::setIndex(std::vector<confToken>	&context)
{
	
}
void	ServerConfig::setRoute(std::vector<confToken>	&context)
{
	
}

ServerConfig::ServerConfig(std::vector<confToken> context): _port(80), _rootDir("/var/www/html"), _indexFile("index.html"), _errorPage()
{

	std::map <std::string, void(ServerConfig::*)(std::vector<confToken> &)> directives;
	directives.insert({std::string("listen"), &ServerConfig::setPort});
	directives.insert({std::string("server_name"), &ServerConfig::setServerNames});
	directives.insert({std::string("location"), &ServerConfig::setRoute});
	directives.insert({std::string("error_pages"), &ServerConfig::setErrorPages});
	directives.insert({std::string("client_body_size"), &ServerConfig::setClientBodySize});
	directives.insert({std::string("default_file"), &ServerConfig::setIndex});
	std::cout<<"Server Tokens:"<<std::endl;
	printConfTokens(context);

}