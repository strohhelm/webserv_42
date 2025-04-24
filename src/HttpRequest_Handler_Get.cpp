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

int HttpRequest::evaluateFilepath(const int& client_fd, std::string& path, ServerConfig& config, routeConfig& route)
{
	if(path.empty())
	{
		if (debug)std::cout << BG_BRIGHT_RED<<"Filepath empty " << RESET<<std::endl;
		sendErrorResponse(client_fd, 404, config);
		return -1;
	}
	int isCgiRequest = checkCgi(path, route);
	if (debug)std::cout << BG_GREEN << "isCgiRequest " << isCgiRequest << RESET << std::endl;
	if(isCgiRequest > 0)
	{
		std::string query = extractQueryString(_requestLine._path);
		_cgi.setCgiParameter(client_fd, config, path, route.getCgiPath(), query);
		_cgi.tokenizePath();
		_cgi.execute("GET", _rawBody);
		return 1;
	}
	else if (isCgiRequest < 0)
	{
		if (debug)std::cout << BG_BRIGHT_RED<<"CGI executable not valid " << RESET<<std::endl;
		sendErrorResponse(client_fd, 500, config);
		return -1;
	}
	return 0;
}

void	HttpRequest::evaluateFiletype(std::string& filename)
{
	std::string fileend = filename.substr(filename.find_last_of('.'));

	std::vector<std::string> endings = {".php", ".html", ".ico"};
	if (std::find(endings.begin(), endings.end(), fileend)!= endings.end())
		_state._websitefile = true;
}



int	HttpRequest::evaluateDownload(const int& client_fd, std::string& path, ServerConfig& config)
{
	if (debug)std::cout << ORANGE<<"Evaluating Downloadmode" << RESET<<std::endl;
	struct stat	fileinfo;
	size_t		filesize;
	std::string	filename;
	filename = path.substr(path.find_last_of('/') + 1);
	_state._downloadFileName = filename;
	if (debug)std::cout << ORANGE<<"Filename: "<< filename << RESET<<std::endl;
	if (stat(path.c_str(), &fileinfo) != 0)
	{
		if (debug)std::cout << BG_BRIGHT_RED<<"stat function error" << RESET<<std::endl;
		sendErrorResponse(client_fd, 500, config);
		return -1;
	}
	filesize = fileinfo.st_size;
	if (debug)std::cout << ORANGE<<"Filesize: "<< filesize << RESET<<std::endl;

	evaluateFiletype(filename);
	if (debug)std::cout << ORANGE<<"Filetype " << MAGENTA<< (_state._downloadMode  == true ? "web":"other") << RESET<<std::endl;

	if (filesize > MAX_SEND_BYTES)
	{

		_state._downloadMode = true;
		_state._downloadFile.open(path, std::ios::in);
		if (!_state._downloadFile.is_open() || _state._downloadFile.bad())
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"File opening error" << RESET<<std::endl;
			sendErrorResponse(client_fd, 500, config);
			return -1;
		}
		_state._downloadSize = filesize;
	}
	if (debug)std::cout << ORANGE<<"Downloadmode " << MAGENTA<< (_state._downloadMode  == true ? "true":"false") << RESET<<std::endl;
	return 0;
}

void	HttpRequest::continueDownload(const int& client_fd)
{
	static int		init = 0;
	static size_t	totalSent = 0;
	char responseBuffer[MAX_SEND_BYTES];

	if (init == 0)
	{
		if (debug)std::cout << ORANGE<<"Starting Download" << RESET<<std::endl;

		std::string response;
		if (_state._websitefile)
			response = buildResponseHeader(200, _state._downloadSize, "text/html");
		else
			response = buildDownloadHeader(200, _state._downloadSize, _state._downloadFileName);
		if (debug)std::cout <<response<<std::endl;
		
		
		size_t bytesSent = send(client_fd, response.c_str(), response.length(), 0);

		if (bytesSent != response.length())
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"Sending header error" << RESET<<std::endl;
			_state._errorOcurred = SEND_ERROR;
			return;
		}
		totalSent += bytesSent;
		init = 1;
	}
	if (init == 1)
	{
		_state._downloadFile.read(responseBuffer, MAX_SEND_BYTES);
		size_t bytesRead = _state._downloadFile.gcount();
		if (_state._downloadFile.bad())
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"File reading error" << RESET<<std::endl;
			_state._errorOcurred = READ_ERROR;
			return;
		}
		size_t bytesSent = send(client_fd, responseBuffer, bytesRead, 0);
		totalSent += bytesRead;
		if (bytesSent != bytesRead)
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"Sending body error" << RESET<<std::endl;
			if (debug)std::cout << BG_BRIGHT_RED<<"BytesRead: "<<bytesRead<<" BytesSent: "<<bytesSent << RESET<<std::endl;
			_state._errorOcurred = SEND_ERROR;
			return;
		}
		if (_state._downloadFile.eof())
		{
			if (debug)std::cout << BG_BRIGHT_GREEN<<"Download successful!" << RESET<<std::endl;
			_state._downloadFile.close();
			init = 0;
			totalSent = 0;
			_state.reset();
		}
	}
	if (debug)std::cout << ORANGE<<"\rStatus: " << RESET << totalSent<< " / "<< _state._downloadSize<<std::endl;
}

void	HttpRequest::singleGetRequest(const int& client_fd, std::string& path, ServerConfig& config, bool isFile)
{
	std::string	content;
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
		if (debug)std::cout << BG_BRIGHT_RED<<"Content empty " << RESET<<std::endl;
		sendErrorResponse(client_fd, 403, config);
		return ;
	}
	sendResponse(client_fd, 200, content);
	_state.reset();
	return ;
}


void HttpRequest::handleGet(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route)
{
	
	//check if we ALREADY HANDLED this request and are in downnloadmode.
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.

	(void)server_fd;
	bool isFile = true;
	std::string path;
	if (!_state._downloadEvaluated)
	{
		_state._downloadEvaluated = true;
		path = getRequestedFile(isFile, config, route);
		if (debug)std::cout << BG_CYAN << "requested file path :" <<RESET<<CYAN<< path << RESET << std::endl;
		if (evaluateFilepath(client_fd, path, config, route) != 0)
			return;
		if (evaluateDownload(client_fd, path, config) != 0)
			return;
	}
	if (_state._downloadMode == true)
	{
		continueDownload(client_fd);
	}
	else
	{
		singleGetRequest(client_fd, path, config, isFile);
	}
	return;
}

