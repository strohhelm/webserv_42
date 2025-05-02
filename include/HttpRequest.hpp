
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
#include <filesystem>
#include "Colors.hpp"
#include <sys/statvfs.h>
#include <limits.h>

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
	bool			_uploadModeEvaluated	= false;
	bool			_uploadMode				= false;
	int				_isCgiPost				= 0;
	bool			_bodyRecieved			= false;
	bool			_uploadComplete			= false;
	
	bool			_downloadEvaluated		= false;
	bool			_downloadMode			= false;
	bool			_downloadComplete		= false;
	size_t			_downloadSize			= 0;
	bool			_websitefile			= true;
	bool			_readyToHandle			= false;
	
	int				_errorOcurred			= 0;
	
	std::string		_tempUploadFilePath;
	std::ofstream	_uploadFile;
	std::string 	_filename;
	std::string		_openBoundary;
	std::string		_closeBoundary;
	std::string		_downloadFileName;
	std::ifstream	_downloadFile;
	std::chrono::steady_clock::time_point _lastActivity;
};

class HttpRequest
{
	private:
	
		requestLine		_requestLine;
		std::string		_rawRequestLine;
		std::unordered_map<std::string, std::string> _headers;
		std::unordered_map<std::string, std::string> _body;
		std::string _httpResponse;
		CGI	_cgi;
		std::string _cgiBuffer;
		//POST
		std::string _cgiFilePath;
		std::string _fileName;
		// std::string body;
		std::string _contentHeader;
		std::string _fileContent;
		std::string _uploadDir;
		std::string _tempDir = "tmp_upload";
		std::string _fdPath;
		std::filesystem::path _path;

		bool _done = false;
	
	
	
		public:
		int				_client_fd;
		routeConfig		*_route;
		ServerConfig	*_config;
		RequestState	_state;


		// HttpRequest(void);
		HttpRequest(int client_fd, ServerConfig* config);

		size_t	extractContentLength(void);
		int		extractAndTokenizeHeader(void);
		int		extractRawRequestLine(void);
		int		tokenizeRequestLine(void);
		void	tokenizeBody(void);
	
		void	setMethod(const std::string& method);
		void	setPath(const std::string& path);
		void	setVersion(const std::string& version);
		//STATE
		void		printState(void);
		int			evaluateState(void);
		void		reset();

		//GET
		int			evaluateDownload(std::string& path);
		void		continueDownload(void);
		void		singleGetRequest(std::string& path);
		int			evaluateFilepath(std::string& path);
		void		evaluateFiletype(std::string& filename);
		//POST
		int			evaluateUpload(void);
		int			checkStorage(void);
		void		handleUpload(void);
		void		cgiPost(void);
		int			extractInfo(void);
		int			dirSetup(void);
		int			extractContent(void);
		int			writeContent(void);
		void		postRespond(void);
		void		checkFilename(std::filesystem::path filePath);
		//VALIDATE
		int			validateRequest(void);
		bool		validateHost(std::vector<std::string> &serverNames);
		int			checkCgi(std::string path);
		//HANDLER
		void		handleHttpRequest();
		void		handleGet(void);
		void		handlePost(void);
		void		handleDelete(void);
		void		handleUnknown(void);
		void		handleForbidden(void);
		void		handleRedirect();

		//SEND
		void		sendErrorResponse(int statusCode);
		void		sendRedirectResponse(int statusCode, const std::string& message);

		HttpMethod	stringToHttpMethod(const std::string& method);
		void		sendResponse(int statusCode, const std::string& message);
		std::string	buildResponseHeader(int statusCode, size_t size, std::string contentType);
		std::string buildDownloadHeader(int statusCode, size_t size, std::string& filename);

		HttpMethod			getMethod(void);
		const std::string&	getPath(void);
		const std::string&	getHttpResponse(void);
		
		const std::string&	getRawRequestLine(void);
		const std::string&	getRawBody(void);

		void	eraseSpaceAndTab(std::string &key, std::string &value);

		void	showHeader(void);
		void	showBody(void);
		std::string	getContentType();
		
		//FILE FINDING
		std::string	getRequestedFile(void);
		std::string	readFileContent(const std::string& path);
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


		// void executeCGI();


};


#endif