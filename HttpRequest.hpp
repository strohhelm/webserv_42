

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
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
		std::string	_headers;
		std::string	_body;
		

		std::unordered_map<std::string, std::string> headers;
		std::unordered_map<std::string, std::string> body;


		requestLine	_requestLine;

		std::string _httpResponse;

	public:
		void	parseHttpRequest(const std::string& requestBuffer);
		void	tokenizeHttpRequest(const std::string& requestBuffer);
		void	tokenizeRequestLine();


		void	setRawRequestLine(std::istringstream& stream);
		void	setHeaders(std::istringstream& stream);
		void	setBody(std::istringstream& stream);

		void	setMethod(const std::string& method);
		void	setPath(const std::string& path);
		void	setVersion(const std::string& version);

		void	clearRequest(void);
		
		
		
		void parseHeaders(const std::string& headerLines);
		bool isValidRequest();
		
		void handleHttpRequest(int fd);
		void handleGET(int fd);
		void handlePOST(void);
		void handleDELETE(void);
		
		
		
		HttpMethod stringToHttpMethod(const std::string& method);
		
		
		
		
		const HttpMethod&	getMethod(void);
		const std::string&	getPath(void);
		const std::string&	getHttpResponse(void);
		
		
		const std::string&	getRawRequestLine(void);
		const std::string&	getHeader(void);
		const std::string&	getBody(void);

		void	eraseSpaceAndTab(std::string key, std::string value);

		std::string	readHtmlFile(const std::string & filename);
		void	serveIndex(int fd);
		void	serveFavicon(int fd);
		void	sendNotFound(int fd);


		void	showHeader(void);
		void	showBody(void);

};


#endif