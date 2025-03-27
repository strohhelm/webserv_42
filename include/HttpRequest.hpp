

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


#include <arpa/inet.h> // send()


enum class HttpMethod
{
	GET,
	POST,
	DELETE,
	UNKNOWN
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
		

		std::unordered_map<std::string, std::string> headers;
		std::unordered_map<std::string, std::string> body;


		requestLine	_requestLine;

		std::string _httpResponse;

	public:
		void	parseHttpRequest(const std::string& requestBuffer);
		void	tokenizeHttpRequest(const std::string& requestBuffer);
		void	tokenizeRequestLine();
		void	extractBody(std::istringstream& stream);
		void	tokenizeBody(const std::string& rawBody);

		void	setRawRequestLine(std::istringstream& stream);
		void	tokenizeHeader(std::istringstream& stream);
		void	setBody(std::istringstream& stream);

		void	setMethod(const std::string& method);
		void	setPath(const std::string& path);
		void	setVersion(const std::string& version);

		void	clearRequest(void);
		
		
		
		void parseHeaders(const std::string& headerLines);
		bool isValidRequest();
		
		void handleHttpRequest(int fd);
		void handleGET(int fd);
		void handlePOST(int fd);
		void handleDELETE(int fd);
		void handleUNKNOWN(int fd);
		void sendErrorResponse(int fd, int statusCode, const std::string& message);
		HttpMethod stringToHttpMethod(const std::string& method);
		
		
		
		
		const HttpMethod&	getMethod(void);
		const std::string&	getPath(void);
		const std::string&	getHttpResponse(void);
		
		
		const std::string&	getRawRequestLine(void);
		const std::string&	getRawBody(void);

		void	eraseSpaceAndTab(std::string key, std::string value);

		void	showHeader(void);
		void	showBody(void);




		std::string getRequestedFile();
		std::string readFileContent(const std::string& path);

		std::string getFileType(const std::string& path);
		std::string getContentType();
		void sendResponse(int fd,const std::string& path, const std::string& message);


};


#endif