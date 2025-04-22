#include "../include/ServerConfig.hpp"

void HttpRequest::sendErrorResponse(int fd, int statusCode, ServerConfig& config)
{
	std::string response;
	std::string content;
	std::string contentType;
	auto it = config._errorPage.find(statusCode);
	if(it != config._errorPage.end())
	{
		
		std::string pathToErrorPage = config._rootDir + "/" + config._errorPage[statusCode];
		content = readFileContent(pathToErrorPage);
		std::cout << "pathToErrorPage " << pathToErrorPage << std::endl;
		std::cout << "content " << content << std::endl;
		contentType = "text/html";
	}
	else
	{
		content = "";
		contentType = "text/plain";
	}
	response = buildResponse(statusCode, StatusCode.at(statusCode), content, contentType);
	// std::string response = buildResponse(statusCode, StatusCode.at(statusCode), StatusCode.at(statusCode), "text/plain");

	send(fd, response.c_str(), response.size(), 0); // return value check!?!?!?!?!?
	_state.reset();
}


void HttpRequest::sendResponse(int fd,int statusCode, const std::string& message)
{
	std::string response = buildResponse(statusCode, "OK" , message, "text/html");

	send(fd, response.c_str(), response.size(), 0);// return value check!?!?!?!?!?
}

std::string HttpRequest::buildResponse(int& statusCode, std::string CodeMessage, const std::string& message, std::string contentType)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + CodeMessage;
	response += "\r\n";
	response += "Content-Length: " + std::to_string(message.size());
	response += "\r\n";
	response += "Content-Type:" + contentType; 
	response += "\r\n\r\n";
	response += message;

	return response;
}