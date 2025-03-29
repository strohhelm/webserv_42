#include "../include/SimpleServer.hpp"
#include "../include/ServerConfig.hpp"
#include "../include/HttpRequest.hpp"


void    printErrorMessage(const std::exception& e)
{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;    
}


int main(void)
{
	// ServerConfig config;

	// config.setUrl()


	try
	{
		SimpleServer server(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, 10);
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}

	return 0;
}