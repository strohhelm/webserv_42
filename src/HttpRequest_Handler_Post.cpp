#include "../include/ServerConfig.hpp"
#include "../include/HttpRequest.hpp"

int HttpRequest::checkStorage(void)
{
	char cwd[PATH_MAX];
	if(getcwd(cwd, sizeof(cwd)) == nullptr)
	{
		if(debug){std::cout << RED << "Couldn't get current working directory" << RESET << std::endl;}
		return (0);
	}
	std::filesystem::space_info space = std::filesystem::space(cwd);
	if (space.available < _state._contentLength)
	{
		if(debug){std::cout << "Not enough space on device." << std::endl;}
		return (0);
	}
	if(debug){std::cout << "Available storage on device: " << space.available << " | Incoming file size: " << _state._contentLength << std::endl;}
	return (1);
}


int HttpRequest::evaluateUpload(void)
{
	if (!_state._uploadEvaluated)
	{
		_state._isCgiPost = checkCgi(_requestLine._path);
		if (_state._isCgiPost < 0)
		{
			sendErrorResponse(500);
			return(0);
		}
		if (_state._isCgiPost > 0 && _state._contentLength > MAX_IN_MEMORY_BODY_SIZE)
		{
			sendErrorResponse(413);
			return(0);
		}
		if ((*_route)._uploadPath.empty() && !_state._isCgiPost)
		{
			sendErrorResponse(401);
			return(0);
		}
		if (!checkStorage())
		{
			sendErrorResponse(507);
			return(0);
		}
		else if (!(*_route)._uploadPath.empty())
			_uploadDir = (*_route)._uploadPath;
		if (_state._contentLength > (*_config)._maxBody)
		{
			sendErrorResponse(413);
			return(0);
		}
		if(!_state._isCgiPost && !dirSetup())
		{
			sendErrorResponse(500);
			return(0);
		}
		_contentHeader = _headers["Content-Type"];
		// _path = _requestLine._path;
		_state._uploadEvaluated = true;
	}
	return (1);
}

void HttpRequest::cgiPost(void)
{

	if (!_state._uploadComplete)
	{
		_cgiBuffer += _state._buffer;
		_state._buffer.clear();
		if (_cgiBuffer.size() == _state._contentLength)
			_state._uploadComplete = true;
		else
			return;
	}
	if (_state._uploadComplete)
	{
		_cgiFilePath = getRequestedFile();
		std::string query = "";
		std::filesystem::path filename = _requestLine._path;
		_cgi.setCgiParameter(_client_fd, (*_config), _cgiFilePath, (*_route).getCgiPath(filename.extension()), query);
		_cgi.tokenizePath();
		int check = _cgi.execute("POST", _cgiBuffer);
		if (check > 0)
			sendErrorResponse(check);
		reset();
		return;
	}
}

void HttpRequest::handlePost(void)
{
	if(debug){std::cout << GREEN << "HANDLE POST" << RESET << std::endl;}
	if (!evaluateUpload())
		return;
	handleUpload();
	if (_state._isCgiPost > 0)
		cgiPost();
}
