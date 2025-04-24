
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "Colors.hpp"

#include "CGI.hpp"
// #include "ServerConfig.hpp"


#include <arpa/inet.h> // send()

class routeConfig;
class ServerConfig;

enum class HttpMethod
{
	GET,
	POST,
	DELETE,
	UNKNOWN,
	FORBIDDEN,
};

enum State
{
	NEEDS_TO_READ,
	NEEDS_TO_WRITE,
	SEND_ERROR,
	READ_ERROR,

};

struct requestLine 
{
	HttpMethod _method = HttpMethod::UNKNOWN;
	std::string _path = "";
	std::string _version = "";
};

struct RequestState
{
	std::string		_buffer;
	int				_isNewRequest			= true;
	int				_isValidRequest			= 0;
	size_t			_ContentBytesRecieved	= 0;
	size_t			_contentLength			= 0;
	bool			_requestlineRecieved	= false;
	bool			_requestlineParsed		= false;

	bool			_headersRecieved		= false;
	bool			_headersParsed			= false;

	bool			_uploadEvaluated		= false;
	bool			_uploadMode				= false;
	
	bool			_bodyRecieved			= false;
	bool			_uploadComplete			= false;

	bool			_downloadEvaluated		= false;
	bool			_downloadMode			= false;
	bool			_downloadComplete		= false;
	size_t			_downloadSize			= 0;
	bool			_websitefile			= false;
	bool			_readyToHandle			= false;
	
	int				_errorOcurred			= 0;

	std::string		_tempUploadFilePath;
	std::ofstream	_uploadFile;
	std::string		_downloadFileName;
	std::ifstream	_downloadFile;
	void	reset();
};

class HttpRequest
{
	private:
	
		std::string		_rawRequestLine;
		std::string		_rawBody;
		
		std::unordered_map<std::string, std::string> _headers;
		std::unordered_map<std::string, std::string> _body;
		
		requestLine	_requestLine;
		
		std::string _httpResponse;
		
		CGI	_cgi;
		routeConfig		*_route;
		ServerConfig	*_config;
		int				_client_fd;
		int				_server_fd;
	// const ServerConfig& _config;
	
	public:
		RequestState	_state;



		HttpRequest(void);
		// int		parseHttpRequest(void);
		// int		clearOldRequest(void);
		size_t	extractContentLength(void);
		int		extractAndTokenizeHeader(void);
		void	extractRawBody(void);
		int		extractRawRequestLine(void);
		int		tokenizeRequestLine(void);
		void	tokenizeBody(void);
	
		void	setMethod(const std::string& method);
		void	setPath(const std::string& path);
		void	setVersion(const std::string& version);
 	
		void		printState(void);
		int			evaluateState(int client_fd);
		int			evaluateDownload(const int& client_fd, std::string& path);
		void		continueDownload(const int& client_fd);
		void		singleGetRequest(const int& client_fd, std::string& path, bool isFile);
		int			evaluateFilepath(const int& client_fd, std::string& path);
		void		evaluateFiletype(std::string& filename);
		int			validateRequest(void);
		bool		validateHost(std::vector<std::string> &serverNames);
		int			checkCgi(std::string path);
		void		handleHttpRequest(const int& client_fd, const int& server_fd);
		void		handleGet(const int& client_fd, const int& server_fd,);
		void		handlePost(const int& client_fd, const int& server_fd,);
		void		handleDelete(int fd, routeConfig &route);
		void		handleUnknown(int fd);
		void		handleForbidden(const int& client_fd);
		void		sendErrorResponse(int fd, int statusCode);
		HttpMethod	stringToHttpMethod(const std::string& method);

		HttpMethod			getMethod(routeConfig &route);
		const std::string&	getPath(void);
		const std::string&	getHttpResponse(void);
		
		const std::string&	getRawRequestLine(void);
		const std::string&	getRawBody(void);

		void	eraseSpaceAndTab(std::string &key, std::string &value);

		void	showHeader(void);
		void	showBody(void);


		std::string	getRequestedFile(bool& isFile,);
		std::string	readFileContent(const std::string& path);

		std::string	getContentType();
		void		sendResponse(int fd,int statusCode, const std::string& message);
		std::string	buildResponseHeader(int statusCode, size_t size, std::string contentType);
		std::string buildDownloadHeader(int statusCode, size_t size, std::string& filename);

		std::string	buildFullPath(void);
		bool		fileExists(const std::string& path);
		bool		directoryExists(const std::string& path);
		std::string	serveDirectory(std::string fullPath);
		int			deleteFile(const std::string& filename);

		/********************************************************/
		const std::unordered_map<std::string, std::string>& getHeaders() const {
			return _headers;
		}

		std::string extractQueryString(std::string& request);


		// void executeCGI(const int& client_fd);


};


#endif