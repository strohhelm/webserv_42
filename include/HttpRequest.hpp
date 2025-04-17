

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

#include"Colors.hpp"

#include"CGI.hpp"
#include"ServerConfig.hpp"


#include <arpa/inet.h> // send()


enum class HttpMethod
{
	GET,
	POST,
	DELETE,
	UNKNOWN,
};

struct requestLine
{
	HttpMethod _method = HttpMethod::UNKNOWN;
	std::string _path = "";
	std::string _version = "";
};

class HttpRequest
{
	private:
		std::string	_rawRequestLine;
		std::string	_rawBody;
		
		std::unordered_map<std::string, std::string> _headers;
		std::unordered_map<std::string, std::string> _body;

		requestLine	_requestLine;

		std::string _httpResponse;

		CGI	_cgi;

	public:
		void parseHttpRequest(const std::string& requestBuffer);
		void clearOldRequest(void);
		void extractAndTokenizeHeader(const std::string& requestBuffer);
		void extractRawBody(const std::string& requestBuffer);
		void extractRawRequestLine(const std::string& requestBuffer);
		void tokenizeRequestLine(void);
		void tokenizeBody(void);
	
		void setMethod(const std::string& method);
		void setPath(const std::string& path);
		void setVersion(const std::string& version);
 	
		int			validateRequest(ServerConfig& config, routeConfig& route);
		bool		validateHost(std::vector<std::string> &serverNames);
		void		handleHttpRequest(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig &route);
		void		handleGet(const int& client_fd, const int& server_fd, ServerConfig& config, routeConfig& route);
		void		handlePost(const int& client_fd, const int& server_fd, ServerConfig& config);
		void		handleDelete(int fd);
		void		handleUnknown(int fd);
		void		sendErrorResponse(int fd, int statusCode, const std::string& message);
		HttpMethod	stringToHttpMethod(const std::string& method);

		HttpMethod	getMethod(routeConfig &route);
		const std::string	getMethodString(void);
		const std::string&	getPath(void);
		const std::string&	getHttpResponse(void);
		
		const std::string&	getRawRequestLine(void);
		const std::string&	getRawBody(void);

		void	eraseSpaceAndTab(std::string key, std::string value);

		void	showHeader(void);
		void	showBody(void);


		std::string getRequestedFile(bool& isFile, ServerConfig& config);
		std::string readFileContent(const std::string& path);

		std::string	getContentType();
		void		sendResponse(int fd,int statusCode, const std::string& message);

		std::string	buildFullPath(ServerConfig& config);
		bool		fileExists(const std::string& path);
		bool		directoryExists(const std::string& path);
		bool		directoryListingIsOff(void);
		std::string	serveDirectory(std::string fullPath);

		int			deleteFile(const std::string& filename);

		/********************************************************/
		const std::unordered_map<std::string, std::string>& getHeaders() const {
			return _headers;
		}
		void runCgiScriptGet(const int& client_fd, const std::string& fullPath);
		void runCgiScriptPost(const int& client_fd, const std::string& fullPath, const std::string& path);


		void executeCGI(const int& client_fd, ServerConfig& config);


};


#endif