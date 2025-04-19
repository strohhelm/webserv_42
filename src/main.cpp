// #include "../include/CGI.hpp"
// #include "../include/HttpRequest.hpp"
#include "../include/SimpleServer.hpp"
// #include "../include/ServerConfig.hpp"
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

std::string getFilename(int argc, char**argv)
{
	std::string filename;
	if (argc > 2)
		throw std::runtime_error("Usage: ./webserv <config_file>");
	else if (argc == 1)
		filename = DEFAULT_CONFIG_PATH;
	else
	{
		if (access(argv[1], F_OK))
			throw std::runtime_error("Cannot access: \"" + std::string(argv[1]) + "\" file not found.");
		else if (access(argv[1], R_OK))
			throw std::runtime_error("Cannot access: \"" + std::string(argv[1]) + "\" permission denied.");
		filename = argv[1];
	}
	return filename;
}

int main(int argc, char **argv)
{
	signal(SIGINT, handleSignalINT);
	try
	{
		std::string	filename = getFilename(argc, argv);
		MainConfig	config(filename);

		myLog(config._error_log, config._access_log);
		SimpleServer server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config._worker_connections, config._http);
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}
	return 0;
}