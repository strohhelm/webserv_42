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

	std::cout << "HANDLE POST" << std::endl;
	if (!_state._uploadEvaluated)
	{
		_state._isCgiPost = checkCgi(_requestLine._path);
		
		if ((*_route)._uploadPath.empty() && !_state._isCgiPost)
		{
			sendErrorResponse(401);// wrong Code
			return;
		}
		if (_state._isCgiPost < 0)
			{sendErrorResponse(500);return;}
		_contentHeader = _headers["Content-Type"];
		_path = _requestLine._path;
		_state._uploadEvaluated = true;
	}
	if (!_state._isCgiPost)
	{
		extractRawBody();
		handleUpload();
		_rawBody.clear();
		return;
	}
	else if (_state._isCgiPost > 0 && _state._uploadMode)
	{
		extractRawBody();
		_cgiBuffer += _rawBody;
		if (_cgiBuffer.size() == _state._contentLength)
			_state._uploadComplete = true;
		else
		{
			_rawBody.clear();
			return;
		}
	}
	if (_state._isCgiPost > 0)
	{
		path = getRequestedFile();
		std::string query = "";
		std::filesystem::path filename = _requestLine._path;
		_cgi.setCgiParameter(_client_fd, (*_config), path, (*_route).getCgiPath(filename.extension()), query);
		_cgi.tokenizePath();
		int check = _cgi.execute("POST", _rawBody);
		if (check > 0)
			{sendErrorResponse(check);return;};
		reset();
		return;
	}
	sendErrorResponse(405);// wrong Code
}

