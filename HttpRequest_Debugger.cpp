#include"HttpRequest.hpp"



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
	for (const auto& [key, value] : headers)
	{
		std::cout << key << ":" << value << "\n";
	}
}

void HttpRequest::showBody(void)
{
	for (const auto& [key, value] : body)
	{
		std::cout << key << "=" << value << "\n";
	}
}