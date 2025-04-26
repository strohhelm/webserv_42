#include "../include/ServerConfig.hpp"



void HttpRequest::handleHttpRequest()
{
	if (debug)std::cout << BG_BRIGHT_RED<<"Route: " << RESET<<std::endl;
	if (debug)std::cout << RED<< (*_route)._methods[0]<<(*_route)._methods[1]<<(*_route)._methods[2] << RESET<<std::endl;

	if (debug)std::cout << BG_BRIGHT_RED<<"Method: " << static_cast<int>(getMethod()) << RESET<<std::endl;
	
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
	//TODO check if allowed to delete!!!!

	std::string path = getPath();
	if (path.front() == '/')
	{
		path.erase(path.begin());
	}
	if (debug)std::cout << BG_BRIGHT_BLACK << "DELETE: "<<RESET<<BLACK<<path << RESET << std::endl;
	if ((*_route)._uploadPath.empty() || path.find((*_route)._uploadPath) != std::string::npos)
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
	sendResponse(204, "Resource deleted successfully");
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

	_uploadEvaluated		= false;
	_uploadMode				= false;
	
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
