#include"../include/HttpRequest.hpp"
#include"../include/Macros.hpp"



/************/
/* Debugger */
/************/

const std::string& HttpRequest::getRawRequestLine(void)
{
	return _rawRequestLine;
}

const std::string& HttpRequest::getRawBody(void)
{
	return _rawBody;
}


void HttpRequest::showHeader(void)
{
	std::cout << BLUE << "headers: " << RESET << std::endl;	
	for (const auto& [key, value] : _headers)
	{
		std::cout << key << "=" << value << "\n";
	}
	std::cout << BLUE << "end of headers" << RESET << std::endl;
}

void HttpRequest::showBody(void)
{
	std::cout << YELLOW << "BODY: " << RESET << std::endl;
	for (const auto& [key, value] : _body)
	{
		std::cout << "[" << key << "]=[" << value << "]\n";
	}
	std::cout << YELLOW << "END OF BODY" << RESET << std::endl;
}