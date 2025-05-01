
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
	else if (type == CLOSE && init)
	{
		error_file<<"SESSION END -- " << ctime(&tt);
		access_file<<"SESSION END -- " << ctime(&tt);
		error_file.close();
		access_file.close();
	}
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
		myLog(config._error_log, config._access_log);
		while (1)
		{
			SimpleServer server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config);
			if (!server._fatalError)
				break;
		}
		myLog(CLOSE, "");
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}
	return 0;
}