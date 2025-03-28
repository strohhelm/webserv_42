#include "../include/SimpleServer.hpp"


void    printErrorMessage(const std::exception& e)
{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;    
}


int main(void)
{

	try
	{
		SimpleServer server(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, 2);
	}
	catch(const std::exception& e)
	{
		printErrorMessage(e);
	}


	return 0;
}