#include "../include/ServerConfig.hpp"

bool HttpRequest::validateHost(std::vector<std::string> &serverNames)
{
	std::string host = _headers["Host"];
	host = host.substr(0, host.find(':'));
	std::cout<<host<<std::endl;
	if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end())
		return true;
	else
		return false;
}

int	HttpRequest::checkCgi(std::string path, routeConfig& route)
{
	std::string filename = path.substr(path.find_last_of("/"), path.size() - path.find_last_of("/"));
	bool check = (filename.substr(filename.size() - 4, filename.size()) == ".php");
	if (check && route.checkCgiPath())
		return 1;
	else if (check)
		return -1;
	return 0;
}

int HttpRequest::validateRequest(ServerConfig& config, routeConfig& route)
{
	std::string path = _requestLine._path;
	auto routes = config._routes;
	if (!validateHost(config._serverNames))
	{
		std::cout << BG_BRIGHT_MAGENTA << "return-1" << RESET << std::endl;
		return -1;
	}
	while (!path.empty())
	{
		auto tmp = routes.find(path);
		if (tmp != routes.end())
		{
			route = tmp->second;
			return 0;
		}
		else
		{
			size_t pos = path.find_last_of('/');
			if (pos == std::string::npos)
			break;
			path = path.substr(0, pos);
		}
	}
	std::cout << BG_BRIGHT_MAGENTA << "return1" << RESET << std::endl;
	// return 1; //originl
	return 0;
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
			handleDelete(client_fd);
			break;
		case HttpMethod::FORBIDDEN:
			handleForbidden(client_fd);
			break;
		default:
			handleUnknown(client_fd);
			break;
	}
}

void HttpRequest::handleGet(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.
	bool isFile = true;
	std::string	content;
	std::string path = getRequestedFile(isFile, config);
	std::cout << BG_CYAN << "path" << RESET << std::endl;
	(void)server_fd;

	if(path.empty())
	{
		sendErrorResponse(client_fd, 404);
		return;
	}
	int check = checkCgi(path, route);
	if(check > 0)
	{
		_cgi.setCgiParameter(client_fd, config, path, route.getCgiPath());
		_cgi.tokenizePath();
		_cgi.execute("GET", _rawBody);
		return;
	}
	else if (check < 0)
		sendErrorResponse(client_fd, 502);

	std::cout << BG_BRIGHT_BLUE << config._rootDir << RESET << std::endl;
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
		sendErrorResponse(client_fd, 403);
		return;
	}
	sendResponse(client_fd, 200, content);	
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
		std::cout << BG_BRIGHT_BLACK << type->second << RESET << std::endl;
		return type->second;
	}
}




void HttpRequest::handleForbidden(const int& client_fd)
{
	sendErrorResponse(client_fd, 403);
}



void HttpRequest::handlePost(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request

	(void)server_fd;


	if(_requestLine._path.find("php") != std::string::npos)
	{	
		_cgi.setCgiParameter(client_fd, config, _requestLine._path, route.getCgiPath());
		_cgi.tokenizePath();
		_cgi.execute("POST", _rawBody);
		return;
	}

	sendErrorResponse(client_fd, 405);// wrong Code 
}


void HttpRequest::handleUnknown(int fd)
{
	sendErrorResponse(fd, 405);
}



int HttpRequest::deleteFile(const std::string& filename)
{
	return (remove(filename.c_str()) == 0);
}


void HttpRequest::handleDelete(int fd)
{
	std::string path = getPath();
	if (path.front() == '/')
	{
		path.erase(path.begin());
	}
	std::cout << BG_BRIGHT_BLACK << path << RESET << std::endl;

	if(!fileExists(path))
	{
		sendErrorResponse(fd, 404);
		return;		
	}
	if(!deleteFile(path))
	{
		sendErrorResponse(fd, 500);
		return;
	}
	sendResponse(fd,204, "Resource deleted successfully");
}
