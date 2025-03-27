#include"../include/HttpRequest.hpp"



/************/
/* Debugger */
/************/

const std::string& HttpRequest::getRawRequestLine(void)
{
	return _rawRequestLine;
}

const std::string& HttpRequest::getHeader(void)
{
	return _headers;
}

const std::string& HttpRequest::getBody(void)
{
	return _body;
}


void HttpRequest::showHeader(void)
{
	std::cout << BLUE << "headers: " << RESET << std::endl;	
	for (const auto& [key, value] : headers)
	{
		std::cout << key << "=" << value << "\n";
	}
	std::cout << BLUE << "end of headers" << RESET << std::endl;
}

void HttpRequest::showBody(void)
{
	std::cout << YELLOW << "BODY: " << RESET << std::endl;
	for (const auto& [key, value] : body)
	{
		std::cout << key << "=" << value << "\n";
	}
	std::cout << YELLOW << "END OF BODY" << RESET << std::endl;
}