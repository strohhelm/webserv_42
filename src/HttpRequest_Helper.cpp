#include"../include/HttpRequest.hpp"


/*
400 Bad Request → Malformed request line, missing Host, or invalid headers.

405 Method Not Allowed → Unsupported HTTP methods.

411 Length Required → Missing Content-Length for POST requests.

414 URI Too Long → Request path too long.
*/


std::string HttpRequest::getRequestedFile()
{
	/*
	TODO
	if paht is file and no directory
		server file
	if is directory and "active dir" flag in config file on
		serve directory
	*/

	if(_requestLine._path == "/")
	{
		/*
		TODO
		get root directory from config
		get index files from config
		check which index file exists
		return root + indexfile
		*/
	return  "www/index.html";
	}
	if(_requestLine._path.find("..") != std::string::npos) //to avoid forbidden access
	return "";
	else
	{
		/*
		TODO
		check if _requestLine.path exists
		get index files from requested path from config
		check which index file exists
		return path + indexfile
		*/
		
		return "www" + _requestLine._path;
	}
}

std::string HttpRequest::readFileContent(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())
		return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}


void HttpRequest::sendErrorResponse(int fd, int statusCode, const std::string& message)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + message + "\r\n";
	response += "Content-Length: " + std::to_string(message.size()) + "\r\n";
	response += "Content-Type: text/plain\r\n\r\n";
	response += message;

	send(fd, response.c_str(), response.size(), 0);
}


void HttpRequest::sendResponse(int fd,int statusCode, const std::string& message)
{

	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " OK\r\n";
	response += "Content-Length: " + std::to_string(message.size()) + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "\r\n";
	response += message;
	
	send(fd, response.c_str(), response.size(), 0);

}
