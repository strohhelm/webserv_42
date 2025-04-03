#include "../include/SimpleServer.hpp"
#include "../include/ServerConfig.hpp"
#include "../include/HttpRequest.hpp"


void    printErrorMessage(const std::exception& e)
{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;    
}


int main(void)
{

	
	std::vector<ServerConfig> configs;
	
	ServerConfig config1;
	std::vector<std::string> hostname1;
	
	ServerConfig config2;
	std::vector<std::string> hostname2;

	hostname1.push_back("example.com");
	hostname1.push_back("www.example.com");
	config1.setUrl(hostname1, 8080);

	hostname2.push_back("test.com");
	hostname2.push_back("www.test.com");
	config2.setUrl(hostname2, 8081);

	configs.push_back(config1);
	configs.push_back(config2);






	try
	{
		SimpleServer server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 10, configs);
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}

	return 0;
}