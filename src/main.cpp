#include "../include/CGI.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/SimpleServer.hpp"
#include "../include/ServerConfig.hpp"
#include "../include/SignalHandler.hpp"


volatile sig_atomic_t g_stopFlag = 0;


static void    printErrorMessage(const std::exception& e)
{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;    
}

static void handleSignalINT(int signal)
{
	(void)signal;
	std::cout << RED << "shutting Server down ..." << RESET << std::endl;
	g_stopFlag = 1;
}


int main(void)
{
	signal(SIGINT, handleSignalINT);

	// std::string cgipath = "/usr/bin/php-cgi";

	std::vector<ServerConfig> configs;
	
	ServerConfig config1;
	std::vector<std::string> hostname1;
	hostname1.push_back("example.com");
	hostname1.push_back("www.example.com");
	config1.setUrl(hostname1, 8080);
	config1.setRootDir("www");
	config1.setCgiPath("/usr/bin/php-cgi");

	

	configs.push_back(config1);


	std::cout << BG_BRIGHT_CYAN << config1.getCgiPath() << RESET << std::endl;
	
	// ServerConfig config2;
	// std::vector<std::string> hostname2;
	// hostname2.push_back("test.com");
	// hostname2.push_back("www.test.com");
	// config2.setUrl(hostname2, 8081);
	// config2.setRootDir("www2");

	// configs.push_back(config2);
	
	// ServerConfig config3;
	// std::vector<std::string> hostname3;
	// hostname3.push_back("test1.com");
	// hostname3.push_back("www.test1.com");
	// config3.setUrl(hostname3, 8082);
	// config3.setRootDir("www3");
	
	// configs.push_back(config3);

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