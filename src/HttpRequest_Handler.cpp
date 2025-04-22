#include "../include/ServerConfig.hpp"



void HttpRequest::handleHttpRequest(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig &route)
{
	switch (getMethod(route))
	{
		case HttpMethod::GET:
			handleGet(client_fd, server_fd, config, route);
			break;
		case HttpMethod::POST:
			handlePost(client_fd, server_fd, config, route);
			break;
		case HttpMethod::DELETE:
			handleDelete(client_fd, config, route);
			break;
		case HttpMethod::FORBIDDEN:
			handleForbidden(client_fd, config);
			break;
		default:
			handleUnknown(client_fd, config);
			break;
	}
}



void HttpRequest::handleForbidden(const int& client_fd, ServerConfig& config )
{
	sendErrorResponse(client_fd, 403, config);
}


void HttpRequest::handleUnknown(int fd, ServerConfig& config)
{
	sendErrorResponse(fd, 405, config);
}


void HttpRequest::handleDelete(int fd, ServerConfig& config, routeConfig &route)
{
	//TODO check if allowed to delete!!!!

	std::string path = getPath();
	if (path.front() == '/')
	{
		path.erase(path.begin());
	}
	if (debug)std::cout << BG_BRIGHT_BLACK << "DELETE: "<<RESET<<BLACK<<path << RESET << std::endl;
	if (route._uploadPath.empty() || path.find(route._uploadPath) != std::string::npos)
		sendErrorResponse(fd, 401, config);
	if(!fileExists(path))
	{
		sendErrorResponse(fd, 404, config);
		return;
	}
	if(!deleteFile(path))
	{
		sendErrorResponse(fd, 500, config);
		return;
	}
	sendResponse(fd, 204, "Resource deleted successfully");
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

	_readyToHandle			= false;
	
	// _tempUploadFilePath.clear();
	// if (_uploadFile.is_open())
	// 	_uploadFile.close;
	// std::string		_tempDownloadFilePath;
	// std::ifstream	_downloadFile;
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
	_requestLine._version.clear();
}
