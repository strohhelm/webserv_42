

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


class HttpRequest
{
	private:
		HttpMethod	_method;
		std::string	_path;
		std::string	_version;
		std::map<std::string, std::string>	_headers;
		std::string	_body;
		// std::map<std::string, std::string>	_queryParams;
		std::string _httpResponse;

	public:
		void parseInput(const std::string& requestBuffer, int fd);
		bool tokenizeRequest(const std::string& requestBuffer,std::string& requestLine, std::string& headers, std::string& body);
		void setMethod(const std::string& method);
		void setPath(const std::string& path);
		const std::string& getPath(void);
		void setVersion(const std::string& version);

		bool parseRequestLine(const std::string& requestLine);
		void parseHeaders(const std::string& headerLines);
		bool isValidRequest();

		void handleRequest(int fd);
		void handleGET(int fd);
		void handlePOST(void);
		void handleDELETE(void);

		const std::string& getHttpResponse(void);


		HttpMethod stringToHttpMethod(const std::string& method);
};


#endif