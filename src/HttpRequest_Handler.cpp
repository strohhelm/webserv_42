#include"../include/HttpRequest.hpp"


void HttpRequest::handleHttpRequest(int fd)
{
	switch (getMethod())
	{
		case HttpMethod::GET:
			handleGET(fd);
			break;
		case HttpMethod::POST:
			handlePOST(fd);
			break;
		case HttpMethod::DELETE:
			handleDELETE(fd);
			break;		
		default:
			handleUNKNOWN(fd);
			break;
	}
}

void HttpRequest::handleGET(int fd)
{
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.

	std::string path = getRequestedFile();
	// std::cout << BG_BRIGHT_BLUE << "path " << path << RESET << std::endl;
	if(path.empty())
	{
		sendErrorResponse(fd, 403, "403 Forbidden");
		return;
	}
	std::string	content = readFileContent(path);
	// std::cout << BG_BRIGHT_BLUE << "content " << content << RESET << std::endl;
	if(content.empty())
	{
		sendErrorResponse(fd, 404, "404 Not Found");
		return;
	}
	sendResponse(fd, path, content);	
}

void HttpRequest::handlePOST(int fd)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request

	std::string response = 
	"HTTP/1.1 404 Not Found\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 13\r\n"
	"Connection: close\r\n\r\n"
	"404 Not Found";

	send(fd, response.c_str(), response.size(), 0);
}

void HttpRequest::handleUNKNOWN(int fd)
{
	sendErrorResponse(fd, 405, "405 Method Not Allowed");
}

void HttpRequest::handleDELETE(int fd)
{
	sendErrorResponse(fd, 101, "101 Delete");
}
