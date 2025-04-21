#include "../include/HttpRequest.hpp"
#include "../include/Macros.hpp"

// Format of the Request Line:
// <HTTP_METHOD> <REQUEST_PATH> <HTTP_VERSION>
// GET /index.html HTTP/1.1

int HttpRequest::tokenizeRequestLine()
{
	std::vector<std::string> tokens;
	std::stringstream stream(_rawRequestLine);
	std::string word;
	int error = 0;

	while(stream>>word)
	{
		tokens.push_back(word);
	}
	if(tokens.size() == 3)
	{
		if (tokens[0].length() > MAX_METHOD_LENGTH)
			return 400;
		setMethod(tokens[0]); 
		setPath(tokens[1]);
		setVersion(tokens[2]);
	}
	else
		error = 400;//if there is more or less than 3 tokens means there is not enough information or whitespace un uri.
	return error;
}

void HttpRequest::extractRawBody()
{
	std::string line;
	std::istringstream stream(_state._buffer);
	while (std::getline(stream, line) && line != "\r"){}
	std::getline(stream, _rawBody, '\0');
}


int HttpRequest::extractRawRequestLine()
{
	// std::istringstream stream(requestBuffer); //andis way
	// std::getline(stream, _rawRequestLine);
	int error = 0;
	size_t pos = _state._buffer.find("\r\n");
	_rawRequestLine = _state._buffer.substr(0, pos);
	_state._buffer = _state._buffer.substr(pos + 2); // +2 to cut \r\n away
	if (_rawRequestLine.empty())
		error = 400;
	if(debug)std::cout<<ORANGE<<"Requestline: "<<RESET<<_rawRequestLine<<std::endl;
	return error;
}

size_t HttpRequest::extractContentLength()
{
	try
	{
		if (_headers.count("ContentLength"))
			return std::stoul(_headers["ContentLength"]);
	}
	catch(...){}
	return 0;
	
}

void HttpRequest::eraseSpaceAndTab(std::string &key, std::string &value)
{
	if (std::isspace(value.front()))
		value.erase(0, 1);
	if (std::isspace(value.back()))
		value.erase(value.length() - 1, 1);
	(void)key;
	// key.erase(key.find_last_not_of(" \t") + 1);
	// value.erase(0, value.find_first_not_of(" \t")); //idk what it does
}

int HttpRequest::extractAndTokenizeHeader()
{
	int error = 0;
	std::string line;
	std::string header = _state._buffer.substr(0, _state._buffer.find("\r\n\r\n"));
	if (header.empty())
		error = 400;
	std::istringstream stream(header);
	while (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			break;
		size_t delimiterPos = line.find(":");
		if(delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);

			std::string value = line.substr(delimiterPos + 1); //space in field syntax is optional : field-line   = field-name ":" OWS field-value OWS
			if (std::isspace(key[0]))
				continue;		//if a field line starts with whitespace, it is to be ignored or send 400 -> rcf 9112 page 7
			eraseSpaceAndTab(key, value);
			_headers[key] = value;
		}
	}
	_state._buffer = _state._buffer.substr(header.length() + 4); //cut the header from the buffer
	if(debug)showHeader();
	return error;
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

// void HttpRequest::clearOldRequest(void)
// {
	// _rawRequestLine.clear();
	// _rawBody.clear();
	// _body.clear();
	// _headers.clear();
	// _requestLine = {}; // Set struct to default values as given in declaration
// }

// void HttpRequest::parseHttpRequest()
// {
	// clearOldRequest();
	// extractRawRequestLine(requestBuffer);
	// tokenizeRequestLine();
	// extractAndTokenizeHeader(requestBuffer);
	// extractRawBody(requestBuffer);
	// tokenizeBody();
	
	// if(isValidRequest() == false)
		// TODO
// }

