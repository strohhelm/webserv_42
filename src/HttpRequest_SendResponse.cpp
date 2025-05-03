#include "../include/ServerConfig.hpp"

void HttpRequest::sendErrorResponse(int statusCode)
{
	std::string response;
	std::string content;
	std::string contentType = "text/html";
	_state._errorOcurred = statusCode;
	if (_config != nullptr)
	{	auto it = (*_config)._errorPage.find(statusCode);
		if(it != (*_config)._errorPage.end())
		{
			std::string pathToErrorPage = (*_config)._rootDir + "/" + (*_config)._errorPage[statusCode];
			content = readFileContent(pathToErrorPage);
		}
	}
	if (content.empty()) //in case the errorpage from config doesnt exist
	{
		std::string message;
		try{ message =  StatusCode.at(statusCode);}
		catch (...){message = "Error";}
		content = "<!DOCTYPE html><html><head><title>" + std::to_string(statusCode) + "</title> \
		<style> \
			@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@500&display=swap'); \
			html, body {height: 100%;margin: 0;} \
			body{font-family: 'Orbitron', sans-serif; \
				display:flex; \
				background-color: #00ffe7; \
				align-items: center; \
				justify-content: center; \
				background-image: radial-gradient(circle at center,rgb(255, 255, 255) 20%, #000000 100%); \
				background-size: cover; \
				background-repeat: no-repeat; \
				background-position: center;} \
		.content{ \
			text-align: center; \
			color:rgb(50, 49, 49); \
			padding: 2rem; \
			text-shadow: 0 0 10px #000000;} \
		</style> \
		</head> \
			<body> \
			<div class=\"content\"> \
				<h1>OOOps an Error occured!</h1> \
				<h2>"+ std::to_string(statusCode) +" " + message + "</h2> \
				<h2>Sorry for the Inconvenience!</h2> \
			</div> \
			</body> \
		</html>";
	}
	response = buildResponseHeader(statusCode,  content.length(), contentType);
	response += content;
	long bytesToSend = response.length();
	long bytesSent = send(_client_fd, response.c_str(), response.size(), 0); 
	if (bytesSent == -1 || bytesSent == 0 || bytesToSend != bytesSent)
	{	
		std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
		_state._errorOcurred = 500;
	}
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
	long bytesToSend = response.length();
	long bytesSent = send(_client_fd, response.c_str(), response.size(), 0);
	if (bytesSent == -1 || bytesSent == 0 || bytesToSend != bytesSent)
	{	
		std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
		_state._errorOcurred = 500;
	}
}

void HttpRequest::sendRedirectResponse(int statusCode, const std::string& message)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + StatusCode.at(statusCode);
	response += "\r\n";
	response += "Content-Type: text/html";
	response += "\r\n";
	response += "Location: " + message;
	response += "\r\n\r\n";
	long bytesToSend = response.length();
	long bytesSent = send(_client_fd, response.c_str(), response.size(), 0);
	if (bytesSent == -1 || bytesSent == 0 || bytesToSend != bytesSent)
	{	
		std::cout<<BG_BRIGHT_RED<<"Error in send function!"<<std::endl;
		_state._errorOcurred = 500;
	}
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