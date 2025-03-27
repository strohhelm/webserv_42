#include"../include/HttpRequest.hpp"


/*
400 Bad Request → Malformed request line, missing Host, or invalid headers.

405 Method Not Allowed → Unsupported HTTP methods.

411 Length Required → Missing Content-Length for POST requests.

414 URI Too Long → Request path too long.
*/


std::string HttpRequest::getRequestedFile()
{
	if(_requestLine._path == "/")
		return  "www/index.html";
	if(_requestLine._path.find("..") != std::string::npos)
		return "";
	return "www" + _requestLine._path;
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
