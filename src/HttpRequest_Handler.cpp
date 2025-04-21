#include "../include/ServerConfig.hpp"

#include "../include/Post.hpp"


bool HttpRequest::validateHost(std::vector<std::string> &serverNames)
{
	std::string host = _headers["Host"];
	host = host.substr(0, host.find(':'));
	if (debug)std::cout<<GREEN<<"HOST: "<<YELLOW<<host<<std::endl;
	if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end())
		return true;
	else
		return false;
}

int	HttpRequest::checkCgi(std::string path, routeConfig& route)
{
	// std::string filename = path.substr(path.find_last_of("/"), path.size() - path.find_last_of("/"));
	// bool check = (filename.substr(filename.size() - 4, filename.size()) == ".php");

	bool endsWithPhp = path.size() >= 4 && path.substr(path.size() - 4) == ".php";

	if (debug)std::cout << ORANGE<<"ends with .php " << (endsWithPhp == true ? "true" : "false") <<RESET<< std::endl;
	if(!endsWithPhp)
		return 0;

	if (endsWithPhp && route.checkCgiPath())
		return 1;
	else 
		return -1;

}

int HttpRequest::validateRequest(ServerConfig& config, routeConfig& route)
{
	if (debug)std::cout<<ORANGE<<"Validating request"<<RESET<<std::endl;
	std::string path = _requestLine._path;
	auto routes = config._routes;
	if (!validateHost(config._serverNames))
	{
		if (debug)std::cout << BG_BRIGHT_MAGENTA << "Host invalid" << RESET << std::endl;
		return -1;
	}
	// does route exist?
	while (!path.empty())
	{
		auto tmp = routes.find(path);
		if (tmp != routes.end())
		{
			route = tmp->second;
			if (debug)std::cout << BG_BRIGHT_MAGENTA << "Request Valid" << RESET << std::endl;
			return 0;
		}
		else
		{
			if (path.find_last_of('/') == path.find_first_of('/'))
			{
				auto it = config._routes.find("/");
				if ( it != config._routes.end())
				{
					route = it->second;
					if (debug)std::cout << BG_BRIGHT_MAGENTA << "Request Valid" << RESET << std::endl;
					return 0;
				}
			}
			size_t pos = path.find_last_of('/');
			if (pos == std::string::npos)
			break;
			path = path.substr(0, pos);
		}
	}
	if (debug)std::cout << BG_BRIGHT_MAGENTA << "Request invalid" << RESET << std::endl;
	return 1;
}


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
			handleDelete(client_fd, config);
			break;
		case HttpMethod::FORBIDDEN:
			handleForbidden(client_fd, config);
			break;
		default:
			handleUnknown(client_fd, config);
			break;
	}
}

std::string HttpRequest::extractQueryString(std::string& request)
{
	size_t pos = request.find('?');
	if(pos != std::string::npos) // only at get
	{
		return request.substr(pos + 1);
	}
	return "";
}
void HttpRequest::handleGet(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	
	//check if we ALREADY HANDLED this request and are in downnloadmode.
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.
	bool isFile = true;
	std::string	content;
	std::string path = getRequestedFile(isFile, config, route);
	if (debug)std::cout << BG_CYAN << "requested file path :" <<RESET<<CYAN<< path << RESET << std::endl;

	(void)server_fd;
	//evaluate downloadMode
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

std::string HttpRequest::getContentType()
{
	auto type = _headers.find("Content-Type");
	
	if(type == _headers.end())
	{
		return "";
	}
	else
	{
		if (debug) std::cout << BG_BRIGHT_BLACK << "Content-Type: "<<RESET<<BLACK<<type->second << RESET << std::endl;
		return type->second;
	}
}

void HttpRequest::handleForbidden(const int& client_fd, ServerConfig& config )
{
	sendErrorResponse(client_fd, 403, config);
}



void HttpRequest::handlePost(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request
	(void)server_fd;

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
		Post p(_requestLine._path, _rawBody, it->second, client_fd);
	}

	// if(getContentType() != "")
	// 	sendErrorResponse(fd, 405, "405 Method Not Allowed");// wrong Code 

	sendErrorResponse(client_fd, 405,config);// wrong Code
	bool isFile = true;

	std::string path = getRequestedFile(isFile, config, route);

	std::string query = "";
	if(_requestLine._path.find("php") != std::string::npos)
	{	
		_cgi.setCgiParameter(client_fd, config, path, route.getCgiPath(), query);
		_cgi.tokenizePath();
		_cgi.execute("POST", _rawBody);
		return;
	}


	sendErrorResponse(client_fd, 405, config);// wrong Code
}


void HttpRequest::handleUnknown(int fd, ServerConfig& config)
{
	sendErrorResponse(fd, 405, config);
}



int HttpRequest::deleteFile(const std::string& filename)
{
	return (remove(filename.c_str()) == 0);
}


void HttpRequest::handleDelete(int fd, ServerConfig& config)
{
	//TODO checki f allowed to delete!!!!
	std::string path = getPath();
	if (path.front() == '/')
	{
		path.erase(path.begin());
	}
	if (debug)std::cout << BG_BRIGHT_BLACK << "DELETE: "<<RESET<<BLACK<<path << RESET << std::endl;

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
	sendResponse(fd,204, "Resource deleted successfully");
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


int	HttpRequest::evaluateState(int client_fd)
{
	if (_state._isNewRequest)
		return NEEDS_TO_READ;
	if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<client_fd<<RESET":"<<std::endl;
	if (!_state._requestlineRecieved)
	{
		if (debug)	std::cout<<YELLOW<<"Requestline not recieved"<<RESET<<std::endl;
		size_t pos = _state._buffer.find("\r\n");
		if (pos == 0)
			pos = _state._buffer.find("\r\n", pos, _state._buffer.length() - pos);
		if (pos != std::string::npos)
			_state._requestlineRecieved = true;
		else
			return NEEDS_TO_READ;
			if (debug)std::cout<<GREEN<<"Requestline recieved"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Requestline already recieved"<<RESET<<std::endl;


	if (!_state._requestlineParsed)
	{
		if (debug)std::cout<<YELLOW<<"Requestline not parsed"<<RESET<<std::endl;
		extractRawRequestLine();
		int error = tokenizeRequestLine();
		if (error)
			return error;
		_state._requestlineParsed = true;
		if (debug)std::cout<<GREEN<<"Requestline parsed"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Requestline already parsed"<<RESET<<std::endl;

	if (!_state._headersRecieved)
	{
		if (debug)std::cout<<YELLOW<<"Headers not recieved"<<RESET<<std::endl;
		size_t pos = _state._buffer.find("\r\n\r\n");
		if (pos != std::string::npos)
			_state._headersRecieved = true;
		else
		{
			if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<client_fd<<BG_BRIGHT_GREEN<<": NEEDS TO READ"<<RESET<<std::endl;
			return NEEDS_TO_READ;
		}
		if (debug)std::cout<<GREEN<<"Headers recieved"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Headers already recieved"<<RESET<<std::endl;
		

	if (!_state._headersParsed)
	{
		if (debug)std::cout<<YELLOW<<"Headers not parsed"<<RESET<<std::endl;

		int error = extractAndTokenizeHeader();
		if (error)
			return error;
		_state._contentLength = extractContentLength();
		if ((!_state._contentLength) && (_requestLine._method == HttpMethod::POST))
			return 400; //check for POST + 0 content length -> chunking
		_state._headersParsed = true;
		if (debug)std::cout<<GREEN<<"Headers parsed"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Headers already parsed"<<RESET<<std::endl;

	if (!_state._uploadEvaluated)
	{
		if (debug)std::cout<<YELLOW<<"Upload not evaluated"<<RESET<<std::endl;

		if (_requestLine._method == HttpMethod::POST && _state._contentLength > MAX_IN_MEMORY_BODY_SIZE)
			_state._uploadMode = true; //chunked request might not have header content-length!! ->transfer encoding
		_state._uploadEvaluated = true;
		if (debug)std::cout<<GREEN<<"Upload evaluated"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Upload already evaluated"<<RESET<<std::endl;

	// if (!_state._downloadEvaluated)
	// {
	// 	if (_requestLine._method == HttpMethod::GET && _state._contentLength > MAX_IN_MEMORY_BODY_SIZE)
	// 		;// _state._downloadMode = true; //find out in  handle get function
	// 	_state._downloadEvaluated = true;
	// }
	switch (_requestLine._method)
	{
		case HttpMethod::POST:
		{
			if (!_state._uploadMode)
			{
				if (_state._buffer.length() < _state._contentLength)
				{
					if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<client_fd<<BG_BRIGHT_GREEN<<": NEEDS TO READ"<<RESET<<std::endl;
					return NEEDS_TO_READ;
				}
				// else
				// 	return NEEDS_TO_WRITE;
			}
			// else
			// {
			// 	// upload(); //validate in post and handle depending on 
			// 	return NEEDS_TO_WRITE;
			// }
			break;
		}
		default:
			;
	}
	if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<client_fd<<BG_BRIGHT_GREEN<<": NEEDS TO WRITE"<<RESET<<std::endl;
	return NEEDS_TO_WRITE;
//think about what happens when one request is handled completely ->reset values
}
