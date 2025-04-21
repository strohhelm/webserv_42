
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

	bool			_readyToHandle			= false;
	
	std::string		_tempUploadFilePath;
	std::ofstream	_uploadFile;
	std::string		_tempDownloadFilePath;
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
 	
		int			evaluateState(int client_fd);
		int			validateRequest(ServerConfig& config, routeConfig& route);
		bool		validateHost(std::vector<std::string> &serverNames);
		int			checkCgi(std::string path, routeConfig& route);
		void		handleHttpRequest(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig &route);
		void		handleGet(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route);
		void		handlePost(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route);
		void		handleDelete(int fd);
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


		std::string	getRequestedFile(bool& isFile, ServerConfig& config, routeConfig& route);
		std::string	readFileContent(const std::string& path);

		std::string	getContentType();
		void		sendResponse(int fd,int statusCode, const std::string& message);
		std::string	buildResponse(int& statusCode, std::string CodeMessage,const std::string& message, std::string contentType);

		std::string	buildFullPath(ServerConfig& config, routeConfig& route);
		bool		fileExists(const std::string& path);
		bool		directoryExists(const std::string& path);
		std::string	serveDirectory(std::string fullPath, ServerConfig& config,routeConfig& route);
		int			deleteFile(const std::string& filename);

		/********************************************************/
		const std::unordered_map<std::string, std::string>& getHeaders() const {
			return _headers;
		}
		void runCgiScriptGet(const int& client_fd, const std::string& fullPath);
		void runCgiScriptPost(const int& client_fd, const std::string& fullPath, const std::string& path);



		// void executeCGI(const int& client_fd, ServerConfig& config);


};


#endif