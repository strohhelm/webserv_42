#include "../include/ServerConfig.hpp"

void HttpRequest::sendErrorResponse(int statusCode)
{
	std::string response;
	std::string content;
	std::string contentType;
	auto it = (*_config)._errorPage.find(statusCode);
	if(it != (*_config)._errorPage.end())
	{
		
		std::string pathToErrorPage = (*_config)._rootDir + "/" + (*_config)._errorPage[statusCode];
		content = readFileContent(pathToErrorPage);
		// if(debug)std::cout << "pathToErrorPage " << pathToErrorPage << std::endl;
		// if(debug)std::cout << "content " << content << std::endl;
		contentType = "text/html";
	}
	else
	{
		if (!(*_route)._redirect.second.empty())
			content = (*_route)._redirect.second;
		else
			content = "";
		contentType = "text/plain";
	}
	response = buildResponseHeader(statusCode,  content.length(), contentType);
	// std::string response = buildResponse(statusCode, StatusCode.at(statusCode), StatusCode.at(statusCode), "text/plain");
	response += content;
	size_t bytesToSend = response.length();
	size_t bytesSent = send(_client_fd, response.c_str(), response.size(), 0); // return value check!?!?!?!?!?
	if (bytesToSend != bytesSent)
		std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
	_state._errorOcurred = statusCode;
	reset();
}


void HttpRequest::sendResponse(int statusCode, const std::string& message)
{
	std::string response;
	if (_state._websitefile)
		response = buildResponseHeader(statusCode , message.length(), "text/html");
	else
		response = buildDownloadHeader(statusCode,  message.length(), _state._downloadFileName);
	if (debug)std::cout<<ORANGE<<"Send Response Headers: "<<RESET<<std::endl<<response<<std::endl;
	response += message;
	size_t bytesToSend = response.length();
	size_t bytesSent = send(_client_fd, response.c_str(), response.size(), 0);// return value check!?!?!?!?!?
	if (bytesToSend != bytesSent)
		std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
}

void HttpRequest::sendRedirectResponse(int statusCode, const std::string& message)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + StatusCode.at(statusCode);
	response += "\r\n";
	response += "Content-Type: text/html";
	response += "\r\n";
	response += "Location: " + message;
	response += "\r\n\r\n";
	size_t bytesToSend = response.length();
	size_t bytesSent = send(_client_fd, response.c_str(), response.size(), 0);
	if (bytesToSend != bytesSent)
		std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
}

std::string HttpRequest::buildResponseHeader(int statusCode, size_t size, std::string contentType)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + StatusCode.at(statusCode);
	response += "\r\n";
	response += "Content-Length: " + std::to_string(size);
	response += "\r\n";
	response += "Content-Type:" + contentType; 
	response += "\r\n\r\n";

	return response;
}

std::string HttpRequest::buildDownloadHeader(int Code, size_t size, std::string& filename)
{
	(void) filename;
	std::string response = "HTTP/1.1 " + std::to_string(Code) + " " + StatusCode.at(Code);
	response += "\r\n";
	response += "Content-Length: " + std::to_string(size);
	response += "\r\n";
	response += "Content-Description: File Transfer";
	response += "\r\n";
	response += "Content-Disposition: attachment; filename=\"" + filename + "\"";
	response += "\r\n";
	response += "Content-Type: application/octet-stream"; 
	response += "\r\n\r\n";

	return response;
}