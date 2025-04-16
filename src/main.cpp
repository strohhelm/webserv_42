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

// for initiation "type" is for error and  "message" is for access file
//Macros ACCESS and ERROR to make it easier to select output
void	myLog(std::string type, std::string message)
{
	static std::ofstream	error_file;
	static std::ofstream	access_file;
	static int				init = 0;
	std::chrono::system_clock::time_point today = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(today);

	if (!init)
	{
		OpenLogFile(type, error_file);
		OpenLogFile(message, access_file);
		error_file<<"SESSION STARTED -- " << ctime(&tt);
		access_file<<"SESSION STARTED -- " << ctime(&tt);
		init = 1;
		return;
	}
	if (type == ACCESS)
		access_file<<ctime(&tt)<<message<<"\n";
	else if (type == ERROR)
		error_file<<ctime(&tt)<<message<<"\n";
	else if (type == CLOSE)
		throw std::runtime_error("Wrong Use of Function myLog!");
}

int main(void)
{
	signal(SIGINT, handleSignalINT);

	// std::string cgipath = "/usr/bin/php-cgi";

	std::vector<ServerConfig> configs;
	
	// ServerConfig config1;
	// std::vector<std::string> hostname1;
	// hostname1.push_back("example.com");
	// hostname1.push_back("www.example.com");
	// config1.setUrl(hostname1, 8080);
	// config1.setRootDir("www");
	// configs.push_back(config1);
	
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
		MainConfig config;
		myLog(config._error_log, config._access_log);
		SimpleServer server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config._worker_connections, config._http);
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}
	return 0;
}