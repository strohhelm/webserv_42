#include "SimpleServer.hpp"




int main(void)
{

	try
	{
		SimpleServer server(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, 2);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}


	return 0;
}