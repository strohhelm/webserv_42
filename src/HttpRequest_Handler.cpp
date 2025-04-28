#include "../include/ServerConfig.hpp"



void HttpRequest::handleHttpRequest()
{
	if ((*_route)._redirect.second.empty())
	{
		switch (getMethod())
		{
			case HttpMethod::GET:
				handleGet();
				break;
			case HttpMethod::POST:
				handlePost();
				break;
			case HttpMethod::DELETE:
				handleDelete();
				break;
			case HttpMethod::FORBIDDEN:
				handleForbidden();
				break;
			default:
				handleUnknown();
				break;
		}
	}
	else
		handleRedirect();
}

void	HttpRequest::handleRedirect()
{
	int code = (*_route)._redirect.first;
	std::string tmp = (*_route)._redirect.second;
	if (code <300)
	{
		_state._websitefile = true;
		if (debug)std::cout << ORANGE <<"Redirect 2xx response" << RESET<<std::endl;	
		sendResponse(code, tmp);
	}
	else if (code < 400)
	{
	if (debug)std::cout << ORANGE <<"Redirect response" << RESET<<std::endl;
		sendRedirectResponse(code, tmp);
	}
	else
	{
	if (debug)std::cout << ORANGE <<"Redirect error response" << RESET<<std::endl;
		sendErrorResponse(code);
	}
	_state.reset();
}


void HttpRequest::handleForbidden()
{
	if (debug)std::cout << BG_BRIGHT_RED<<"Forbidden Method" << RESET<<std::endl;
	sendErrorResponse(403);
}


void HttpRequest::handleUnknown(void)
{
	if (debug)std::cout << BG_BRIGHT_RED<<"Unkown Method" << RESET<<std::endl;
	sendErrorResponse(405);
}


void HttpRequest::handleDelete(void)
{

	std::string path = getPath();
	if (path.front() == '/')
	{
		path.erase(path.begin());
	}
	if (debug)std::cout << BG_BRIGHT_BLACK << "DELETE: "<<RESET<<BLACK<<path << RESET << std::endl;
	if ((*_route)._uploadPath.empty() || path.find((*_route)._uploadPath) == std::string::npos)
		sendErrorResponse(401);
	
	if(!fileExists(path))
	{
		sendErrorResponse(404);
		return;
	}
	if(!deleteFile(path))
	{
		sendErrorResponse(500);
		return;
	}
	sendResponse(202, "Resource deleted successfully");
	_state.reset();
}


void RequestState::reset()
{
	_buffer.clear();
	_isNewRequest			= true;
	_isValidRequest			= 0;
	_ContentBytesRecieved	= 0;
	_contentLength			= 0;
	_requestlineRecieved	= false;
	_requestlineParsed		= false;

	_headersRecieved		= false;
	_headersParsed			= false;

	_uploadModeEvaluated	= false;
	_uploadMode				= false;
	_isCgiPost				= 0;
	_bodyRecieved			= false;
	_uploadComplete			= false;

	_downloadEvaluated		= false;
	_downloadMode			= false;
	_downloadComplete		= false;
	_downloadSize			= 0;
	_websitefile			= false;
	_readyToHandle			= false;
	
	_errorOcurred			= 0;
	
	if (_uploadFile.is_open())
		_uploadFile.close();
	_uploadFile.clear();
	
	if (_downloadFile.is_open())
		_downloadFile.close();
	_downloadFile.clear();

	_tempUploadFilePath.clear();
	_downloadFileName.clear();
	if (debug)std::cout<<ORANGE<<"State reset"<<RESET<<std::endl;
}

HttpRequest::HttpRequest()
{
	_state._buffer.clear();
	_rawRequestLine.clear();
	_rawBody.clear();
	_headers.clear();
	_body.clear();
	_httpResponse.clear();
	_requestLine._path.clear();
	_requestLine._method = HttpMethod::UNKNOWN;
	_requestLine._version.clear();
}
