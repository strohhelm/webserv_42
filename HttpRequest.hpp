

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

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
	HttpMethod	_method;
	std::string	_path;
	std::string	_version;
};

class HttpRequest
{
	private:
		
		std::string	_rawRequestLine;
		std::string	_headers;
		std::string	_body;
		
		requestLine	_requestLine;

		std::string _httpResponse;


	public:
		void parseInput(const std::string& requestBuffer);
		void tokenizeHttpRequest(const std::string& requestBuffer);
		void tokenizeRequestLine();


		void setRawRequestLine(std::istringstream& stream);
		void setHeaders(std::istringstream& stream);
		void setBody(std::istringstream& stream);

		void setMethod(const std::string& method);
		void setPath(const std::string& path);
		const std::string& getPath(void);
		void setVersion(const std::string& version);

		void parseHeaders(const std::string& headerLines);
		bool isValidRequest();

		void handleRequest(int fd);
		void handleGET(int fd);
		void handlePOST(void);
		void handleDELETE(void);

		const std::string& getHttpResponse(void);


		HttpMethod stringToHttpMethod(const std::string& method);




		const HttpMethod& getMethod(void);

};


#endif