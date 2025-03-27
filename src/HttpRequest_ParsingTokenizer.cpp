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

void HttpRequest::extractBody(std::istringstream& stream)
{
	std::string line;
	while (std::getline(stream, line) && line != "\r"){}
	std::getline(stream, _rawBody, '\0');	
}


void HttpRequest::tokenizeHttpRequest(const std::string& requestBuffer)
{
	std::istringstream requestLineStream(requestBuffer);
	std::istringstream headerStream(requestBuffer);
	std::istringstream bodyStream(requestBuffer);

	setRawRequestLine(requestLineStream);
	tokenizeHeader(headerStream);

	extractBody(bodyStream);
	std::cout << "rawbody :" << _rawBody << std::endl;
	tokenizeBody(_rawBody);
}

void HttpRequest::setRawRequestLine(std::istringstream& stream)
{
	std::getline(stream, _rawRequestLine);
}


void HttpRequest::eraseSpaceAndTab(std::string key, std::string value)
{
	key.erase(key.find_last_not_of(" \t") + 1);
	value.erase(0, value.find_first_not_of(" \t"));
}


void HttpRequest::tokenizeHeader(std::istringstream& stream)
{
	std::string line;
	while (std::getline(stream, line))
	{
		size_t delimiterPos = line.find(":");
		if(delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 2);
			eraseSpaceAndTab(key, value);
			headers[key] = value;
		}
	}
}


void HttpRequest::setBody(std::istringstream& stream)
{
	std::string line;
	bool messageFound = false;

	while (std::getline(stream, line))
	{
		if (!messageFound && line.find("message") != std::string::npos)
		{
			messageFound = true;
		}		
		if (messageFound)
		{
			size_t delimiterPos = line.find("=");
			if (delimiterPos != std::string::npos)
			{
				std::string key = line.substr(0, delimiterPos);
				std::string value = line.substr(delimiterPos + 1);
				eraseSpaceAndTab(key, value);
				body[key] = value;
			}
		}
	}
}


void HttpRequest::tokenizeBody(const std::string& rawBody)
{
	std::istringstream stream(rawBody);
	std::string pair;

	while (getline(stream, pair, '&'))
	{
		size_t delimiterPos = pair.find("=");
		if (delimiterPos != std::string::npos)
		{
			std::string key = pair.substr(0, delimiterPos);
			std::string value = pair.substr(delimiterPos + 1);
			eraseSpaceAndTab(key, value);
			body[key] = value;
		}
	}
}



void HttpRequest::clearRequest(void)
{
	_rawRequestLine.clear();

	std::cout << "before clear\n" << _rawBody << "\n" << std::endl;
	
	_rawBody.clear();
	body.clear();
	
	std::cout << "after clear\n"<< "[" << _rawBody << "]\n" << std::endl;


	_requestLine = {}; // Set struct to default values as given in declaration
}

void HttpRequest::parseHttpRequest(const std::string& requestBuffer)
{
	clearRequest();
	tokenizeHttpRequest(requestBuffer);
	tokenizeRequestLine();
	// validate ???
}

