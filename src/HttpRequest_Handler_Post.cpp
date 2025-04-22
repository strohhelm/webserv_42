#include "../include/ServerConfig.hpp"
#include "../include/Post.hpp"


void HttpRequest::handlePost(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request
	(void)server_fd;

	if (debug)
	extractRawBody();
	{std::cout << "POST request incoming" << std::endl;
	std::cout << "---" << std::endl;
	std::cout << "Requestline:" <<_rawRequestLine << std::endl;
	std::cout << "Body: " <<_rawBody << std::endl;
	std::cout << "Path: " <<_requestLine._path << std::endl;
	std::cout << "---" << std::endl;}
	
	auto it = _headers.find("Content-Type");
	if (it != _headers.end())
	{
		std::cout << it->second << std::endl;
		// Post p(_requestLine._path, _rawBody, it->second, client_fd);
	}

	// if(getContentType() != "")
	// 	sendErrorResponse(fd, 405, "405 Method Not Allowed");// wrong Code 

	sendErrorResponse(client_fd, 405,config);// wrong Code
	bool isFile = true;

	std::string path = getRequestedFile(isFile, config, route);

	std::string query = "";
	if(_requestLine._path.find("php") != std::string::npos)
	{	
		_cgi.setCgiParameter(client_fd, config, path, route.getCgiPath(), query);
		_cgi.tokenizePath();
		_cgi.execute("POST", _rawBody);
		return;
	}


	sendErrorResponse(client_fd, 405, config);// wrong Code
}