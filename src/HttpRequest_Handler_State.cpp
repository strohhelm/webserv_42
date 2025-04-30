#include "../include/ServerConfig.hpp"


void	HttpRequest::printState(void)
{
	std::stringstream print;
	print<<CYAN<<"_downloadEvaluated: "<< (_state._downloadEvaluated ==true? "true": "false") <<RESET<<std::endl;
	print<<CYAN<<"_downloadMode: "<< (_state._downloadMode ==true? "true": "false") <<RESET<<std::endl;
	print<<CYAN<<"_websitefile: "<< (_state._websitefile ==true? "true": "false") <<RESET<<std::endl;
	print<<CYAN<<"_uploadFile: "<< _state._downloadFileName <<RESET<<std::endl;
	print<<CYAN<<"Method: "<< static_cast<int>(_requestLine._method)  <<RESET;
	// print<<CYAN<<": "<< (_state. ==true? "true": "false") <<RESET<<std::endl;
	std::cout<<print.str()<<std::endl;
}
int	HttpRequest::evaluateState(void)
{
	//if there is no bytes recieved on this conection after the last request it will not loop and look through the buffer.
	//this flag is set by the readfromclient function as soon as the first byte is recieved for a new request
	if (_state._isNewRequest){return NEEDS_TO_READ;}
	if (debug){std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<_client_fd<<RESET":"<<std::endl;}
	// if (debug){printState();}
	if (!_state._requestlineRecieved)
	{
		if (debug)	std::cout<<YELLOW<<"Requestline not recieved"<<RESET<<std::endl;
		size_t pos = _state._buffer.find("\r\n"); //cheching if there is enough bytes recieved that the statusline is complete
		if (pos == 0) //there might be a \r\n at the start of the bytestring. that is allowed and to be ignored in case. -> rcf 9112
			pos = _state._buffer.find("\r\n", pos, _state._buffer.length() - pos);
		if (pos != std::string::npos)
			_state._requestlineRecieved = true;
		else
			return NEEDS_TO_READ;

		if (debug)std::cout<<GREEN<<"Requestline recieved"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Requestline already recieved"<<RESET<<std::endl;



	//if the requestline was recieved but not yet parsed, it will go here
	if (!_state._requestlineParsed)
	{
		if (debug)std::cout<<YELLOW<<"Requestline not parsed"<<RESET<<std::endl;
		extractRawRequestLine();
		int error = tokenizeRequestLine(); //returns errors in case of requestline being malformed, also cuts the requestline from the buffer incl the linefeed
		if (error)
			return error;
		_state._requestlineParsed = true;
		if (debug)std::cout<<GREEN<<"Requestline parsed"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Requestline already parsed"<<RESET<<std::endl;



	//checks if all headers are recieved
	if (!_state._headersRecieved)
	{
		if (debug)std::cout<<YELLOW<<"Headers not recieved"<<RESET<<std::endl;
		size_t pos = _state._buffer.find("\r\n\r\n");
		if (pos != std::string::npos)
			_state._headersRecieved = true;
		else
		{
			if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<_client_fd<<BG_BRIGHT_GREEN<<": NEEDS TO READ"<<RESET<<std::endl;
			return NEEDS_TO_READ;
		}
		if (debug)std::cout<<GREEN<<"Headers recieved"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Headers already recieved"<<RESET<<std::endl;


		//checks if the headers are already parsed.
	if (!_state._headersParsed)
	{
		if (debug)std::cout<<YELLOW<<"Headers not parsed"<<RESET<<std::endl;

		int error = extractAndTokenizeHeader(); //also cuts the headers from the buffer
		if (error)
			return error; //returns in caser something is wrong in the headers.
		_state._contentLength = extractContentLength();
		if ((!_state._contentLength) && (_requestLine._method == HttpMethod::POST))
			return 411; //check for POST + 0 content length -> chunking
		if (_state._contentLength > (*_config)._maxBody)
			return 413;
		_state._headersParsed = true;
		if (debug)std::cout<<GREEN<<"Headers parsed"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Headers already parsed"<<RESET<<std::endl;


	//evaluates if the incoming content is too long to keep in the buffer in memory.
	//in that case it will set _uoploadMode to true so POST handler knows what to do
	if (!_state._uploadModeEvaluated)
	{
		if (debug)std::cout<<YELLOW<<"Upload not evaluated"<<RESET<<std::endl;
		if (_requestLine._method == HttpMethod::POST && _state._contentLength > MAX_IN_MEMORY_BODY_SIZE)
		_state._uploadMode = true; //chunked request might not have header content-length!! ->transfer encoding
		_state._uploadModeEvaluated = true;
		if (debug)std::cout<<GREEN<<"Upload evaluated"<<RESET<<std::endl;
	}
	else
		if (debug)std::cout<<GREEN<<"Upload already evaluated"<<RESET<<std::endl;


	// not neccessary the switch statement for now, but might want to include bnehaviour for other methods,
	// that need to be evaluated. could be shortened for final version
	switch (_requestLine._method)
	{
		case HttpMethod::POST:
		{
			// if the request body is small enough to keep in memory, it will read the full request into the buffer.
			// only then the handle_post will get the request.
			// in case of uploadmode, handle_post gets every buffer chunk and has to stream,
			// it temporarily into a file until it is completely recieved.
			if (!_state._uploadMode)
			{
				if (_state._buffer.length() < _state._contentLength)
				{
					if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<_client_fd<<BG_BRIGHT_GREEN<<": NEEDS TO READ"<<RESET<<std::endl;
					return NEEDS_TO_READ;
				}
				// else
				// 	return NEEDS_TO_WRITE;
			}
			// else
			// {
			// 	// upload(); //validate in post and handle depending on 
			// 	return NEEDS_TO_WRITE;
			// }
			break;
		}
		default:
			;
	}
	if (debug)std::cout<<BG_BRIGHT_YELLOW<<UNDERLINE<<BOLD<<"STATE for client on FD "<<BG_BRIGHT_MAGENTA<<_client_fd<<BG_BRIGHT_GREEN<<": NEEDS TO WRITE"<<RESET<<std::endl;
	return NEEDS_TO_WRITE;
}