#include "../include/ServerConfig.hpp"
#include "../include/Post.hpp"

void HttpRequest::evaluateUpload(void)
{
	if (!_state._uploadEvaluated)
	{
		_state._isCgiPost = checkCgi(_requestLine._path);
		if ((*_route)._uploadPath.empty() && !_state._isCgiPost)
		{
			sendErrorResponse(401);// wrong Code
			return;
		}
		else if (!(*_route)._uploadPath.empty())
			_uploadDir = (*_route)._uploadPath;
		if (_state._isCgiPost < 0)
		{
			sendErrorResponse(500);
			return;
		}
		if (_state._contentLength > (*_config)._maxBody)
		{
			sendErrorResponse(413);
			return;
		}
		if(!_state._isCgiPost && !dirSetup())
		{
			sendErrorResponse(500);
			return;
		}
		_contentHeader = _headers["Content-Type"];
		// _path = _requestLine._path;
		_state._uploadEvaluated = true;
	}
}

void HttpRequest::cgiPost(void)
{
	if (_state._isCgiPost > 0)
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
			_cgi.execute("POST", _cgiBuffer);
			reset();
			return;
		}
	}
}

void HttpRequest::handlePost(void)
{
	std::cout << "HANDLE POST" << std::endl;
	evaluateUpload();
	handleUpload();
	cgiPost();
}
