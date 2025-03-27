#include"../include/HttpRequest.hpp"


// Format of the Request Line:
// <HTTP_METHOD> <REQUEST_PATH> <HTTP_VERSION>
// GET /index.html HTTP/1.1

void HttpRequest::tokenizeRequestLine()
{
	std::vector<std::string> tokens;
	std::stringstream stream(_rawRequestLine);
	std::string line;

	while(getline(stream, line, ' '))
	{
		tokens.push_back(line);
	}

	if(tokens.size() == 3)
	{
		setMethod(tokens[0]);
		setPath(tokens[1]);
		setVersion(tokens[2]);			
	}
}

void HttpRequest::extractRawBody(const std::string& requestBuffer)
{
	std::string line;
	std::istringstream stream(requestBuffer);
	while (std::getline(stream, line) && line != "\r"){}
	std::getline(stream, _rawBody, '\0');	
}


void HttpRequest::extractRawRequestLine(const std::string& requestBuffer)
{
	std::istringstream stream(requestBuffer);
	std::getline(stream, _rawRequestLine);
}


void HttpRequest::eraseSpaceAndTab(std::string key, std::string value)
{
	key.erase(key.find_last_not_of(" \t") + 1);
	value.erase(0, value.find_first_not_of(" \t"));
}


void HttpRequest::extractAndTokenizeHeader(const std::string& requestBuffer)
{
	std::string line;
	std::istringstream stream(requestBuffer);

	while (std::getline(stream, line))
	{
		size_t delimiterPos = line.find(":");
		if(delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 2);
			eraseSpaceAndTab(key, value);
			_headers[key] = value;
		}
	}
}

void HttpRequest::tokenizeBody()
{
	std::istringstream stream(_rawBody);
	std::string pair;

	while (getline(stream, pair, '&'))
	{
		size_t delimiterPos = pair.find("=");
		if (delimiterPos != std::string::npos)
		{
			std::string key = pair.substr(0, delimiterPos);
			std::string value = pair.substr(delimiterPos + 1);
			eraseSpaceAndTab(key, value);
			_body[key] = value;
		}
	}
}

void HttpRequest::clearOldRequest(void)
{
	_rawRequestLine.clear();
	_rawBody.clear();
	_body.clear();
	_headers.clear();
	_requestLine = {}; // Set struct to default values as given in declaration
}

bool HttpRequest::isValidRequest(void)
{
	return true;
}

void HttpRequest::parseHttpRequest(const std::string& requestBuffer)
{
	clearOldRequest();
	extractRawRequestLine(requestBuffer);
	tokenizeRequestLine();
	extractAndTokenizeHeader(requestBuffer);
	extractRawBody(requestBuffer);
	tokenizeBody();
	
	// if(isValidRequest() == false)
		// TODO
}

