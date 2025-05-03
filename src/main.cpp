
#include "../include/SimpleServer.hpp"

#include "../include/SignalHandler.hpp"


volatile sig_atomic_t g_stopFlag = 0;
bool debug = false;

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

std::string getFilename(int argc, char**argv)
{
	std::string	filename;
	if (argc > 3)
		throw std::runtime_error("Usage: ./webserv <config_file> <flag \"-d\">");
	else if (argc == 1)
		filename = std::string(DEFAULT_CONFIG_PATH);
	else if (argc == 2)
	{
		if (std::string(argv[1]) == "-d")
		{
			debug = true;
			filename = std::string(DEFAULT_CONFIG_PATH);
		}
		else
			filename = std::string(argv[1]);
	}
	else if (argc == 3)
	{
		if (std::string(argv[1]) == "-d")
		{
			debug = true;
			filename = argv[2];
		}
		else if (std::string(argv[2]) == "-d")
		{
			debug = true;
			filename = std::string(argv[1]);
		}
		else 
			throw std::runtime_error("Usage: only config file path and optional -d flag");
	}
	if (access(filename.c_str(), F_OK))
		throw std::runtime_error("Cannot access: \"" + filename + "\" file not found.");
	else if (access(filename.c_str(), R_OK))
		throw std::runtime_error("Cannot access: \"" + filename + "\" permission denied.");
	return filename;
}

int main(int argc, char **argv)
{
	signal(SIGINT, handleSignalINT);
	try
	{
		std::string	filename = getFilename(argc, argv);
		MainConfig	config(filename);
		if (debug)std::cout<<BG_BRIGHT_MAGENTA<<"DEBUG MODE"<<RESET<<std::endl;
		while (1)
		{
			SimpleServer server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config);
			if (!server._fatalError)
				break;
		}
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}
	return 0;
}