#include"../include/HttpRequest.hpp"

#include <sys/stat.h> //stat
#include <filesystem>

/*
400 Bad Request → Malformed request line, missing Host, or invalid headers.

405 Method Not Allowed → Unsupported HTTP methods.

411 Length Required → Missing Content-Length for POST requests.

414 URI Too Long → Request path too long.
*/


bool HttpRequest::fileExists(const std::string& path)
{
	
	std::cout << "file exists function path " << path << std::endl;
	struct stat buffer;
	if(stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode))
	{
		std::cout << "file exists" << std::endl;
		return true;
	}
	std::cout << "file exists not" << std::endl;
	return false;
	
}


bool HttpRequest::directoryExists(const std::string& path)
{
	struct stat buffer;
	if(stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{
		std::cout << "dir exists" << std::endl;
		return true;
	}
	std::cout << "dir exists not" << std::endl;
	return false;
}

bool HttpRequest::directoryListingIsOff()
{
	// check for config File
	return false;
}

std::string HttpRequest::serveDirectory(std::string fullPath)
{
	std::stringstream html;
	// std::cout << "serving Directory" << std::endl;
	if(directoryListingIsOff())
		return"";

	html << "<!DOCTYPE html>\n"
		 <<	"<html>\n"
		 << "<head>\n"
		 <<	"	<title> Directory Listing for " << "</title>\n"
		 << "</head>\n"
		 <<	"<body>\n"
		 << "<h1>\n" << " Directory Listing for " << fullPath << "</h1>\n";

	for (const auto& entry : std::filesystem::directory_iterator(fullPath))
	{
		std::cout << entry.path().filename() << std::endl;
		html << "<p>" << entry.path().filename();
	}
	html << "</p>";
		
	html <<	"</body>\n"
		 <<	"</html>\n";




	return html.str();
}

std::string HttpRequest::buildFullPath(ServerConfig& config)
{
	std::string _rootDir = config.getRootDir(); // extract from config file object
	std::string fullPath = _rootDir + _requestLine._path;
	if(_requestLine._path == "/")
	{
		return fullPath + "index.html"; //extract from config. if 2 indexes are availiable check all and give first that fits?
	}
	if(fullPath.find("..") != std::string::npos) //to avoid forbidden access
	{
		return "";
	}
	return fullPath;
}


std::string HttpRequest::getRequestedFile(bool& isFile, ServerConfig& config)
{
	std::string fullPath = buildFullPath(config);
	if(fileExists(fullPath) && !directoryExists(fullPath))
		return(fullPath);
	if(directoryExists(fullPath))
	{
		isFile = false;
		return(serveDirectory(fullPath));
	}
	return "";
}

std::string HttpRequest::readFileContent(const std::string& path)
{
	std::cout << "path:::\n" << path << std::endl;
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())
	{
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}


void HttpRequest::sendErrorResponse(int fd, int statusCode, const std::string& message)
{
	// std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + message;
	// response += "\r\n";
	// response += "Content-Length: " + std::to_string(message.size());
	// response += "\r\n";
	// response += "Content-Type: text/plain";
	// response += "\r\n\r\n";
	// response += message;
	
	std::string temp = message;
	std::string response = buildResponse(statusCode, temp, message);

	send(fd, response.c_str(), response.size(), 0); // return value check!?!?!?!?!?
}


void HttpRequest::sendResponse(int fd,int statusCode, const std::string& message)
{

	// std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " OK";
	// response += "\r\n";
	// response += "Content-Length: " + std::to_string(message.size());
	// response += "\r\n";
	// response += "Content-Type: text/html";
	// response += "\r\n\r\n";
	// response += message;
	std::string message2 = "OK";
	std::string response = buildResponse(statusCode, message2 , message);

	send(fd, response.c_str(), response.size(), 0);// return value check!?!?!?!?!?

}

std::string HttpRequest::buildResponse(int& statusCode, std::string& CodeMessage,const std::string& message)
{
	std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + CodeMessage;
	response += "\r\n";
	response += "Content-Length: " + std::to_string(message.size());
	response += "\r\n";
	response += "Content-Type: text/plain";
	response += "\r\n\r\n";
	response += message;

	return response;
}