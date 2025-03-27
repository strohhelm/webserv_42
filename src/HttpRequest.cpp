#include"../include/HttpRequest.hpp"


/*
400 Bad Request → Malformed request line, missing Host, or invalid headers.

405 Method Not Allowed → Unsupported HTTP methods.

411 Length Required → Missing Content-Length for POST requests.

414 URI Too Long → Request path too long.
*/

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

std::string HttpRequest::getRequestedFile()
{
	std::string path;

	if(_requestLine._path == "/")
		path = "/index.html";

	if(path.find("..") != std::string::npos)
		return "";

	return "www" + path;
}

std::string HttpRequest::readFileContent(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())
		return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void HttpRequest::handleGET(int fd)
{
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.

	std::string path = getRequestedFile();
	if(path.empty())
	{
		sendErrorResponse(fd, 403, "403 Forbidden");
		return;
	}
	std::string	content = readFileContent(path);
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


void HttpRequest::sendErrorResponse(int fd, int statusCode, const std::string& message)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + message + "\r\n";
	response += "Content-Length: " + std::to_string(message.size()) + "\r\n";
	response += "Content-Type: text/plain\r\n\r\n";
	response += message;

	send(fd, response.c_str(), response.size(), 0);
}

std::string HttpRequest::getContentType(const std::string& path)
{
	std::cout << RED << "path " << path << RESET << std::endl;
	if(path.find(".ico"))
		return "image/x-icon";
	if(path.find(".html"))
		return "text/html";
	std::cout << "wrong path" << std::endl;
	return "";
}

void HttpRequest::sendResponse(int fd,const std::string& path, const std::string& message)
{
	(void)path;
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Length: " + std::to_string(message.size()) + "\r\n";
	response += "Content-Type: text/html\r\n";
	// response += "Content-Type: " + getContentType(path) + "\r\n";
	response += "\r\n";
	response += message;
	
	send(fd, response.c_str(), response.size(), 0);

}
