#include "../include/ServerConfig.hpp"
#include "../include/Post.hpp"


void HttpRequest::handlePost(void)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request

	// extractRawBody();
	// if (debug)
	// {std::cout << "POST request incoming" << std::endl;
	// std::cout << "---" << std::endl;
	// std::cout << "Requestline:" <<_rawRequestLine << std::endl;
	// std::cout << "Body: " <<_rawBody << std::endl;
	// std::cout << "Path: " <<_requestLine._path << std::endl;
	// std::cout << "---" << std::endl;}
	
	// auto it = _headers.find("Content-Type");
	// if (it != _headers.end())
	// {
	// 	std::cout << it->second << std::endl;
	// 	// Post p(_requestLine._path, _rawBody, it->second, client_fd);
	// }

	// if(getContentType() != "")
	// 	sendErrorResponse(fd, 405, "405 Method Not Allowed");// wrong Code


	if (!_state._uploadEvaluated)
	{
		std::string path = _requestLine._path;
		_state._isCgiPost = checkCgi(path);
		
		if ((*_route)._uploadPath.empty())
		{
			sendErrorResponse(401);// wrong Code
			return;
		}
		if (_state._isCgiPost < 0)
			sendErrorResponse(500);
		_state._uploadEvaluated = true;
	}
	if (_state._uploadMode && !_state._uploadComplete)
	{
		//open file and stream content
		//tokens?
		return;
	}

	if (_state._isCgiPost)
	{
		extractRawBody();
		std::string query = "";
		if(_state._isCgiPost > 0)
		{
			std::filesystem::path filename = _requestLine._path;
			_cgi.setCgiParameter(_client_fd, (*_config), _requestLine._path, (*_route).getCgiPath(filename.extension()), query);
			_cgi.tokenizePath();
			_cgi.execute("POST", _rawBody);
			return;
		}
	}
	
	sendErrorResponse(405);// wrong Code
}

