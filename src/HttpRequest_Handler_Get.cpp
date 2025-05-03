#include "../include/ServerConfig.hpp"



int HttpRequest::evaluateFilepath(std::string& path)
{
	if(path.empty())
	{
		if (debug)std::cout << RED<<"Filepath empty " << RESET<<std::endl;
		sendErrorResponse((_state._errorOcurred != 0 ?_state._errorOcurred : 404));
		return -1;
	}
	int isCgiRequest = checkCgi(path);
	if (debug)std::cout << BG_GREEN << "isCgiRequest " << isCgiRequest << RESET << std::endl;
	
	if(isCgiRequest > 0)
	{
		std::string query = extractQueryString(_requestLine._path);
		std::filesystem::path filename = (path.substr(0, path.find('?')));
		_cgi.setCgiParameter(_client_fd, (*_config), path, (*_route).getCgiPath(filename.extension()), query);
		_cgi.tokenizePath();
		int check = _cgi.execute("GET", "");
		if (check > 0)
			sendErrorResponse(check);
		reset();
		return 1;
	}
	else if (isCgiRequest < 0)
	{
		if (debug)std::cout << BG_BRIGHT_RED<<"CGI executable not valid " << RESET<<std::endl;
		sendErrorResponse(500);
		return -1;
	}
	return 0;
}

void	HttpRequest::evaluateFiletype(std::string& filename)
{
	std::string fileend = filename.substr(filename.find_last_of('.'));

	 if (fileend == ".pdf" || fileend == ".txt" || fileend.empty() )
	 	_state._websitefile = false;
}



int	HttpRequest::evaluateDownload(std::string& path)
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
		sendErrorResponse(500);
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
			sendErrorResponse(500);
			return -1;
		}
		_state._downloadSize = filesize;
	}
	if (debug)std::cout << ORANGE<<"Downloadmode " << MAGENTA<< (_state._downloadMode  == true ? "true":"false") << RESET<<std::endl;
	return 0;
}

void	HttpRequest::continueDownload()
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
		
		
		long bytesSent = send(_client_fd, response.c_str(), response.length(), 0);
		if (bytesSent == -1 || bytesSent == 0 || bytesSent != (long)response.length())
		{	
			std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
			_state._errorOcurred = 500;
			return ;
		}
		init = 1;
	}
	if (init == 1)
	{
		_state._downloadFile.read(responseBuffer, MAX_SEND_BYTES);
		long bytesRead = _state._downloadFile.gcount();
		if (_state._downloadFile.bad())
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"File reading error" << RESET<<std::endl;
			_state._errorOcurred = READ_ERROR;
			return;
		}
		long bytesSent = send(_client_fd, responseBuffer, bytesRead, 0);
		totalSent += bytesRead;
		if (bytesSent == -1 || bytesSent == 0 || bytesSent != bytesRead)
		{
			if (debug)std::cout << BG_BRIGHT_RED<<"Sending body error" << RESET<<std::endl;
			if (debug)std::cout << BG_BRIGHT_RED<<"BytesRead: "<<bytesRead<<" BytesSent: "<<bytesSent << RESET<<std::endl;
			_state._errorOcurred = SEND_ERROR;
			return;
		}
		std::cout << ORANGE<<"\rStatus: " << RESET << totalSent<< " / "<< _state._downloadSize 
		<< " ["
		<< std::string((totalSent * 50) / _state._downloadSize, '=') 
		<< std::string(50 - (totalSent * 50) / _state._downloadSize, ' ') 
		<< "]" << std::flush;
		if (_state._downloadFile.eof())
		{
			if (debug)std::cout << BG_BRIGHT_GREEN<<"Download successful!" << RESET<<std::endl;
			_state._downloadFile.close();
			init = 0;
			totalSent = 0;
			reset();
		}
	}
}

void	HttpRequest::singleGetRequest(std::string& path)
{
	if (debug)std::cout << BG_BRIGHT_BLUE <<"Root directory:"<<RESET<<BLUE<< (*_config)._rootDir << RESET << std::endl;
	
	std::string	content;
	if(debug)std::cout <<ORANGE<< "State Error:" << RESET<<_state._errorOcurred << std::endl;

	content = readFileContent(path);
	
	if(_state._errorOcurred == 0 && content.empty())
	{
		if (debug)std::cout << BG_BRIGHT_RED<<"Content empty " << RESET<<std::endl;
		sendErrorResponse(403);
		return ;
	}
	else if (_state._errorOcurred != 0)
	{
		if (debug)std::cout << BG_BRIGHT_RED<<"ERROR opening temp dirlisting file!" << RESET<<std::endl;
		sendErrorResponse(500);
		return ;
	}
	if(debug)std::cout <<ORANGE<< "State Error:" << RESET<<_state._errorOcurred << std::endl;
	if(debug)std::cout <<ORANGE<< "Send Response" << RESET << std::endl;

	sendResponse(200, content);
	reset();
	if (path == "./temp/dirListing.html")
	{
		std::filesystem::path p = path;
		if (std::filesystem::exists(p))
			std::filesystem::remove(p);
	}
	return ;
}


void HttpRequest::handleGet()
{

	std::string path;
	if (!_state._downloadEvaluated)
	{
		_state._downloadEvaluated = true;
		path = getRequestedFile();
		if (debug)std::cout << BG_CYAN << "requested file path :" <<RESET<<CYAN<< path << RESET << std::endl;
		if (evaluateFilepath(path) != 0)
			return;
		if (evaluateDownload(path) != 0)
			return;
	}
	if (_state._downloadMode == true)
	{
		continueDownload();
	}
	else
	{
		singleGetRequest(path);
	}
	return;
}
