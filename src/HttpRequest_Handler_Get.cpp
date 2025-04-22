#include "../include/ServerConfig.hpp"


std::string HttpRequest::extractQueryString(std::string& request)
{
	size_t pos = request.find('?');
	if(pos != std::string::npos) // only at get
	{
		return request.substr(pos + 1);
	}
	return "";
}

void HttpRequest::evaluateDownload(const int& client_fd, std::string& path, ServerConfig& config, routeConfig& route)
{
	if(path.empty())
	{
		sendErrorResponse(client_fd, 404, config);
		return;
	}
	int isCgiRequest = checkCgi(path, route);
	if (debug)std::cout << BG_GREEN << "isCgiRequest " << isCgiRequest << RESET << std::endl;
	if(isCgiRequest > 0)
	{
		std::string query = extractQueryString(_requestLine._path);
		_cgi.setCgiParameter(client_fd, config, path, route.getCgiPath(), query);
		_cgi.tokenizePath();
		_cgi.execute("GET", _rawBody);
		return;
	}
	else if (isCgiRequest < 0)
	{
		sendErrorResponse(client_fd, 500, config);
		return;
	}
}


void HttpRequest::handleGet(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	
	//check if we ALREADY HANDLED this request and are in downnloadmode.
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.

	(void)server_fd;
	bool isFile = true;
	std::string	content;
	std::string path;
	if (!_state._downloadEvaluated)
	{
		
		path = getRequestedFile(isFile, config, route);
		if (debug)std::cout << BG_CYAN << "requested file path :" <<RESET<<CYAN<< path << RESET << std::endl;
	}
	else
		evaluateDownload();

	
	//evaluate downloadMode


	if (debug)std::cout << BG_BRIGHT_BLUE <<"Root directory:"<<RESET<<BLUE<< config._rootDir << RESET << std::endl;
	// std::cout << BG_BRIGHT_BLUE << "path " << path << RESET << std::endl;
	if(isFile)
	{
		content = readFileContent(path);
	}
	else
	{
		content = path;
	}
	if(content.empty())
	{
		sendErrorResponse(client_fd, 403, config);
		return;
	}
	sendResponse(client_fd, 200, content);
	_state.reset();
}

