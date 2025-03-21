#include"HttpRequest.hpp"

void HttpRequest::parseInput(const std::string& requestBuffer, int fd)
{
	std::string requestLine;
	std::string headers;
	std::string body;

	tokenizeRequest(requestBuffer, requestLine, headers, body);

	std::cout << RED << "requestLine: " << requestLine << RESET << std::endl;
	std::cout << BLUE << "headers: " << headers << RESET << std::endl;
	std::cout << YELLOW << "body: " << body << RESET << std::endl;

	parseRequestLine(requestLine);
	handleRequest(fd);
}


// Format of the Request Line:
// <HTTP_METHOD> <REQUEST_PATH> <HTTP_VERSION>
// GET /index.html HTTP/1.1

bool HttpRequest::parseRequestLine(const std::string& requestLine)
{
	std::vector<std::string> tokens;
	std::stringstream stream(requestLine);
	std::string line;

	while(getline(stream, line, ' '))
	{
		// std::cout << BG_BRIGHT_MAGENTA << line << RESET << std::endl;
		tokens.push_back(line);
	}

	if(tokens.size() == 3)
	{
		setMethod(tokens[0]);
		setPath(tokens[1]);
		setVersion(tokens[2]);			
	}

	for(int i = 0; i < 3; i++)
	{
		std::cout << BG_BRIGHT_CYAN << tokens[i] << RESET << std::endl;
	}

	return 0;
}

bool HttpRequest::tokenizeRequest(const std::string& requestBuffer, std::string& requestLine, std::string& headers, std::string& body)
{
	std::istringstream stream(requestBuffer);
	std::string line;
	bool isHeaderSection = true;

	// std::cout << GREEN << "tokenizeRequest" << RESET << std::endl;

	if (std::getline(stream, requestLine)) {
		// std::cout << RED << "requestLine: " << requestLine << RESET << std::endl;
	}

	while (std::getline(stream, line)) {
		if (line.empty()) {  // Empty line indicates end of headers
			isHeaderSection = false;
			continue;
		}

		if (isHeaderSection) {
			headers.append(line + "\n");
		} else {
			body.append(line + "\n");
		}
	}

	// std::cout << BLUE << "headers: " << headers << RESET << std::endl;
	// std::cout << YELLOW << "body: " << body << RESET << std::endl;

	return true;
}

const std::string& HttpRequest::getPath(void)
{
	return _path;
}

HttpMethod HttpRequest::stringToHttpMethod(const std::string& method)
{
	if(method == "GET") return HttpMethod::GET;
	if(method == "POST") return HttpMethod::POST;
	if(method == "DELETE") return HttpMethod::DELETE;
	return HttpMethod::UNKNOWN;
}

void HttpRequest::setMethod(const std::string& method)
{
	_method = stringToHttpMethod(method);
}

void HttpRequest::setPath(const std::string& path)
{
	_path = path;
}

void HttpRequest::setVersion(const std::string& version)
{
	_version = version;
}


void HttpRequest::handleRequest(int fd)
{
	// switch (_method)
	// {
	// 	case HttpMethod::GET:
	// 		handleGET(fd);
	// 		break;
	// 	case HttpMethod::POST:
	// 		handlePOST();
	// 		break;
	// 	case HttpMethod::DELETE:
	// 		handleDELETE();
	// 		break;		
	// 	default:
	// 		break;
	// }
}

std::string readHtmlFile(const std::string & filename)
{
	std::ifstream file(filename);
	if(!file)
		return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}


void HttpRequest::handleGET(int fd)
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

	// write(_client_fd, httpResponse.c_str(), httpResponse.size());
	// close(_client_fd);



	// sendHtmlFile("index.html")

	// sendFavicon();

	// sendResponse(404, "NotFound");
}

void HttpRequest::handlePOST(void)
{
	
}

void HttpRequest::handleDELETE(void)
{

}


const std::string& HttpRequest::getHttpResponse(void)
{
	return _httpResponse;
}
