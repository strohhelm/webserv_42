#include"../include/HttpRequest.hpp"



void HttpRequest::handleHttpRequest(const int& client_fd, const int& server_fd, ServerConfig& config)
{
	switch (getMethod())
	{
		case HttpMethod::GET:
			handleGet(client_fd, server_fd, config);
			break;
		case HttpMethod::POST:
			handlePost(client_fd, server_fd, config);
			break;
		case HttpMethod::DELETE:
			handleDelete(client_fd);
			break;		
		default:
			handleUnknown(client_fd);
			break;
	}
}

void HttpRequest::handleGet(const int& client_fd, const int& server_fd, ServerConfig& config)
{
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.
	bool isFile = true;
	std::string	content;
	std::string path = getRequestedFile(isFile, config);
	(void)server_fd;

	std::cout << BG_BRIGHT_BLUE << "-> " << _requestLine._path.find("php") << RESET << std::endl;
	

	if(_requestLine._path.find("php") != std::string::npos)
	{	
		std::string fullPath = config.getRootDir() + _requestLine._path;
		runCgiScriptGet(client_fd, fullPath);
		
		// executeCGI(client_fd, config);
		return;
	}

	std::cout << BG_BRIGHT_BLUE << config.getRootDir() << RESET << std::endl;
	// std::cout << BG_BRIGHT_BLUE << "path " << path << RESET << std::endl;
	if(path.empty())
	{
		sendErrorResponse(client_fd, 404, "404 Not Found");
		return;
	}
	if(isFile)
	{
		content = readFileContent(path);
	}
	else
	{
		content = path;
	}
	
	// std::cout << BG_BRIGHT_BLUE << "content [" << content << "]" << RESET << std::endl;
	
	if(content.empty())
	{
		sendErrorResponse(client_fd, 403, "403 Forbidden");
		return;
	}
	sendResponse(client_fd, 200, content);	
}

std::string HttpRequest::getContentType()
{
	auto type = _headers.find("Content-Type");
	
	if(type == _headers.end())
	{
		return "";
	}
	else
	{
		std::cout << BG_BRIGHT_BLACK << type->second << RESET << std::endl;
		return type->second;
	}
}




void HttpRequest::handlePost(const int& client_fd, const int& server_fd, ServerConfig& config)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request

	(void)server_fd;

	if(_requestLine._path.find("php") != std::string::npos)
	{	
		
		std::string fullPath = config.getRootDir() + _requestLine._path;
		runCgiScriptPost(client_fd, fullPath, _requestLine._path);
		return;
	}

	sendErrorResponse(client_fd, 405, "405 NO CGI");// wrong Code 
}

void HttpRequest::handleUnknown(int fd)
{
	sendErrorResponse(fd, 405, "405 Method Not Allowed");
}


int HttpRequest::deleteFile(const std::string& filename)
{
	return (remove(filename.c_str()) == 0);
}



void HttpRequest::handleDelete(int fd)
{
	std::string path = getPath();
	if (path.front() == '/') {
		path.erase(path.begin());  // Remove the leading "/"
	}
	std::cout << BG_BRIGHT_BLACK << path << RESET << std::endl;

	if(!fileExists(path))
	{
		sendErrorResponse(fd, 404, "404 Not Found");
		return;		
	}
	if(!deleteFile(path))
	{
		sendErrorResponse(fd, 500, "500 Failed to delete resource");
		return;
	}
	sendResponse(fd,204, "Resource deleted successfully");


}
