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
	reset();
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
	reset();
}


void HttpRequest::reset()
{
	_state._buffer.clear();
	_state._isNewRequest			= true;
	_state._isValidRequest			= 0;
	_state._ContentBytesRecieved	= 0;
	_state._contentLength			= 0;
	_state._requestlineRecieved	= false;
	_state._requestlineParsed		= false;
	_state._headersRecieved		= false;
	_state._headersParsed			= false;
	_state._uploadEvaluated		= false;
	_state._uploadModeEvaluated	= false;
	_state._uploadMode				= false;
	_state._isCgiPost				= 0;
	_state._bodyRecieved			= false;
	_state._uploadComplete			= false;
	_state._downloadEvaluated		= false;
	_state._downloadMode			= false;
	_state._downloadComplete		= false;
	_state._downloadSize			= 0;
	_state._websitefile			= true;
	_state._readyToHandle			= false;
	_state._errorOcurred			= 0;
	if (_state._uploadFile.is_open())
		_state._uploadFile.close();
	_state._uploadFile.clear();
	
	if (_state._downloadFile.is_open())
		_state._downloadFile.close();
	_state._downloadFile.clear();

	_state._tempUploadFilePath.clear();
	_state._downloadFileName.clear();
	_state._filename.clear();
	_state._openBoundary.clear();
	_state._closeBoundary.clear();
	_headers.clear();
	_requestLine._path.clear();
	_requestLine._version.clear();
	_rawRequestLine.clear();
	_body.clear();
	_httpResponse.clear();
	_cgiFilePath.clear();
	_contentHeader.clear();
	_fileContent.clear();
	_uploadDir = "file_upload";
	_tempDir = "tmp_upload";
	_fdPath.clear();
	_path.clear();
	if (debug)std::cout<<ORANGE<<"State reset"<<RESET<<std::endl;
}

HttpRequest::HttpRequest()
{
	_state._buffer.clear();
	_rawRequestLine.clear();
	_headers.clear();
	_body.clear();
	_httpResponse.clear();
	_requestLine._path.clear();
	_requestLine._method = HttpMethod::UNKNOWN;
	_requestLine._version.clear();
}
