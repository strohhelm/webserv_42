#include"HttpRequest.hpp"

void HttpRequest::sendNotFound(int fd)
{
	std::string response = 
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"Connection: close\r\n\r\n"
		"404 Not Found";

	send(fd, response.c_str(), response.size(), 0);
}


void HttpRequest::serveFavicon(int fd)
{
	std::ifstream file("favicon.ico", std::ios::binary);
	if (!file) {
		sendNotFound(fd);
		return;
	}

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			<< "Content-Type: image/x-icon\r\n"
			<< "Content-Length: " << file.seekg(0, std::ios::end).tellg() << "\r\n"
			<< "Connection: close\r\n\r\n";
	
	file.seekg(0, std::ios::beg);
	std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	response.write(buffer.data(), buffer.size());

	send(fd, response.str().c_str(), response.str().size(), 0);
}


std::string HttpRequest::readHtmlFile(const std::string & filename)
{
	std::ifstream file(filename);
	if(!file)
		return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}


void HttpRequest::serveIndex(int fd)
{
	std::string htmlContent = readHtmlFile("index.html");
	if(htmlContent.empty())
	{
		_httpResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
	}
	else
	{
		_httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
					std::to_string(htmlContent.size()) + "\r\n\r\n" + htmlContent;
	}
	send(fd, _httpResponse.c_str(), _httpResponse.size(), 0);
}


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

void HttpRequest::tokenizeHttpRequest(const std::string& requestBuffer)
{
	std::istringstream requestLineStream(requestBuffer);
	std::istringstream headerStream(requestBuffer);
	std::istringstream bodyStream(requestBuffer);

	setRawRequestLine(requestLineStream);
	setHeaders(headerStream);
	setBody(bodyStream);
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


void HttpRequest::setHeaders(std::istringstream& stream)
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
			std::cout << "message found" << std::endl;
			messageFound = true;
		}		
		if (messageFound)
		{
			size_t delimiterPos = line.find("=");
			if (delimiterPos != std::string::npos)
			{
				std::string key = line.substr(0, delimiterPos);
				std::string value = line.substr(delimiterPos + 1); // Skip the '=' character
				eraseSpaceAndTab(key, value);  // Clean up spaces and tabs from key and value
				body[key] = value;  // Store key-value pair in the body map
			}
		}
	}
}



void HttpRequest::clearRequest(void)
{
	_rawRequestLine.clear();
	_headers.clear();
	_body.clear();
	_requestLine = {}; // Set struct to default values as given in declaration
}

void HttpRequest::parseHttpRequest(const std::string& requestBuffer)
{
	clearRequest();
	tokenizeHttpRequest(requestBuffer);
	tokenizeRequestLine();
	// validate ???
}

HttpMethod HttpRequest::stringToHttpMethod(const std::string& method)
{
	if(method == "GET") return HttpMethod::GET;
	if(method == "POST") return HttpMethod::POST;
	if(method == "DELETE") return HttpMethod::DELETE;
	return HttpMethod::UNKNOWN;
}

void HttpRequest::handleHttpRequest(int fd)
{
	switch (getMethod())
	{
		case HttpMethod::GET:
			handleGET(fd);
			break;
		case HttpMethod::POST:
			handlePOST();
			break;
		case HttpMethod::DELETE:
			handleDELETE();
			break;		
		default:
			break;
	}
}


void HttpRequest::handleGET(int fd)
{
	if (_requestLine._path == "/favicon.ico") {
		serveFavicon(fd);
		return;
	}
	else if (_requestLine._path == "/" || _requestLine._path == "/index.html") {
		serveIndex(fd);
		return;
	}
	else
		sendNotFound(fd);
}

void HttpRequest::handlePOST(void)
{
	std::cout << "POST is requested" << std::endl;
}

void HttpRequest::handleDELETE(void)
{

}



/*******************/
/* Getter & Setter */
/*******************/

const std::string& HttpRequest::getPath(void)
{
	return _requestLine._path;
}

void HttpRequest::setPath(const std::string& path)
{
	_requestLine._path = path;
}

void HttpRequest::setMethod(const std::string& method)
{
	_requestLine._method = stringToHttpMethod(method);
}

const HttpMethod& HttpRequest::getMethod(void)
{
	return _requestLine._method;
}

void HttpRequest::setVersion(const std::string& version)
{
	_requestLine._version = version;
}

const std::string& HttpRequest::getHttpResponse(void)
{
	return _httpResponse;
}



/************/
/* Debugger */
/************/

const std::string& HttpRequest::getRawRequestLine(void)
{
	return _rawRequestLine;
}

const std::string& HttpRequest::getHeader(void)
{
	return _headers;
}

const std::string& HttpRequest::getBody(void)
{
	return _body;
}


void HttpRequest::showHeader(void)
{
	for (const auto& [key, value] : headers)
	{
		std::cout << key << ":" << value << "\n";
	}
}

void HttpRequest::showBody(void)
{
	for (const auto& [key, value] : body)
	{
		std::cout << key << "=" << value << "\n";
	}
}