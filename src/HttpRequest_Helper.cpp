#include"../include/ServerConfig.hpp"

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
	auto it = path.rbegin();
	if(*it == '/' && stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{
		std::cout << "dir exists" << std::endl;
		return true;
	}
	std::cout << "dir exists not" << std::endl;
	return false;
}


std::string HttpRequest::serveDirectory(std::string fullPath, ServerConfig& config, routeConfig& route)
{
	std::stringstream html;
	(void)config;
	if((!route.isDirListingActive()))
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

/*
	size_t pos = _requestPath.find('?');
	if(pos != std::string::npos) // only at get
	{
		_scriptPath = _requestPath.substr(0, pos); // "/index2.php"
		_queryString = _requestPath.substr(pos + 1); // "name=Alice&lang=de"
	}
	else // only at Post
	{
		_scriptPath = _requestPath;
	}
*/


std::string HttpRequest::buildFullPath(ServerConfig& config, routeConfig& route)
{
	(void)config;
	std::string rootDir = route.getRootDir();
	std::string path = _requestLine._path;
	std::string fullPath;
	
	size_t pos = path.find('?');
	if (pos != std::string::npos)
	{
		path = path.substr(0, pos);
	}
	fullPath = rootDir + path;

	std::cout << "fullPath " << fullPath << std::endl;

	auto it = _requestLine._path.rbegin();
	if(*it == '/')
	{
		std::cout << "ends with /" << std::endl;
		// are there a default files and does one of them exist?
		for(auto it : route._defaultFile)
		{
			std::cout << it << std::endl;
		}
		for(auto it : route._defaultFile)
		{
			std::cout << "it "<< it << std::endl;
			std::string temp = fullPath + it;
			if(access(temp.c_str(), F_OK) == 0)// read access?
			{
				fullPath += it; //extract from config. if 2 indexes are availiable check all and give first that fits?
				break;
			} 
		}
		// return fullPath; //extract from config. if 2 indexes are availiable check all and give first that fits?
	}
	if(fullPath.find("..") != std::string::npos) //to avoid forbidden access
	{
		return "";
	}
	return fullPath;
}


std::string HttpRequest::getRequestedFile(bool& isFile, ServerConfig& config,routeConfig& route)
{
	std::string fullPath = buildFullPath(config, route);
	std::cout << "fullPath " << fullPath << std::endl;
	if(fileExists(fullPath) && !directoryExists(fullPath))
		return(fullPath);
	if(directoryExists(fullPath))
	{
		isFile = false;
		return(serveDirectory(fullPath, config, route));
	}
	return "";
}

std::string HttpRequest::readFileContent(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())
	{
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}


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