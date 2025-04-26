#include "../include/ServerConfig.hpp"
#include "../include/Post.hpp"


void HttpRequest::handlePost(void)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request

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

	sendErrorResponse(405);// wrong Code

	std::string path = getRequestedFile();

	std::string query = "";
	if(_requestLine._path.find("php") != std::string::npos)
	{	
		_cgi.setCgiParameter(_client_fd, (*_config), path, (*_route).getCgiPath(), query);
		_cgi.tokenizePath();
		_cgi.execute("POST", _rawBody);
		return;
	}


	sendErrorResponse(405);// wrong Code
}