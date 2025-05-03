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

int HttpRequest::extractRawRequestLine()
{

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
		if (_headers.count("Content-Length"))
			return std::stoul(_headers["Content-Length"]);
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
}

int HttpRequest::extractAndTokenizeHeader()
{
	try{
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
		size_t delimiterPos = line.find_first_of(':');
		if(delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);

			std::string value = line.substr(delimiterPos + 1); //space in field syntax is optional : field-line   = field-name ":" OWS field-value OWS
			if (std::isspace(key[0]))
				continue;		//if a field line starts with whitespace, it is to be ignored or send 400 -> rcf 9112 page 7
			eraseSpaceAndTab(key, value);
			if (key == "Host" && (_headers.count("Host") == 1))
				{std::cout<<BG_BRIGHT_RED<<"Oh shit wrong Host Header"<<RESET
					<<std::endl; for (auto i:_headers){std::cout<<BLUE<<i.first<<" -> "<<i.second<<RESET<<std::endl;}return 400;}
			else if( (key == "Content-Type") && (_headers.count("Content-Type") == 1))
				{std::cout<<BG_BRIGHT_RED<<"Oh shit Content-Type headers"<<RESET<<std::endl;std::cout<<header<<std::endl;return 400;}
			else if ((key == "Content-Length") && (_headers.count("Content-Length") == 1) && (_headers["Content-Length"] != value))
				{std::cout<<BG_BRIGHT_RED<<"Oh shit wrong Content-Length header"<<RESET<<std::endl;std::cout<<header<<std::endl;return 400;}

			_headers[key] += value;
		}
	}
	if(debug){std::cout<<ORANGE<<"Headers:\n"<<RESET<<header<<std::endl;}
	if (!_headers.count("Host"))
		{std::cout<<BG_BRIGHT_RED<<"Oh shit no Host header"<<RESET<<std::endl;return 400;}
	if (_headers["Connection"] != "keep-alive")
		_state._errorOcurred = 1;
	_state._buffer = _state._buffer.substr(header.length() + 4); //cut the header from the buffer
	return error;
	}catch(...){std::cout<<RED<<"extract Header failed (prob substr)"<<std::endl;return 500;}
}
