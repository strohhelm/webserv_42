#include"../include/ServerConfig.hpp"

#include <filesystem>

/*
400 Bad Request → Malformed request line, missing Host, or invalid headers.

405 Method Not Allowed → Unsupported HTTP methods.

411 Length Required → Missing Content-Length for POST requests.

414 URI Too Long → Request path too long.
*/
int HttpRequest::deleteFile(const std::string& filename)
{
	return (remove(filename.c_str()) == 0);
}


std::string HttpRequest::getContentType()
{
	auto type = _headers.find("Content-Type");
	
	if(type == _headers.end())
	{
		return "";
	}
	else
	{
		if (debug) std::cout << BG_BRIGHT_BLACK << "Content-Type: "<<RESET<<BLACK<<type->second << RESET << std::endl;
		return type->second;
	}
}


bool HttpRequest::fileExists(const std::string& path)
{
	
	if (debug)std::cout << ORANGE<<"File " << path;
	struct stat buffer;
	if(stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode))
	{
		if (debug)std::cout << " exists" <<RESET<< std::endl;
		return true;
	}
	if (debug)std::cout << " exists not" << RESET<<std::endl;
	return false;
}


bool HttpRequest::directoryExists(const std::string& path)
{
	if (debug)std::cout << ORANGE<<"Directory " << path;
	struct stat buffer;
	auto it = path.rbegin();
	if(*it == '/' && stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{
		if (debug)std::cout << " exists" << RESET<<std::endl;
		return true;
	}
	if (debug)std::cout << " exists not" << RESET<<std::endl;
	return false;
}


std::string HttpRequest::serveDirectory(std::string fullPath)
{
	if((!(*_route).isDirListingActive()))
		return"";
	std::string tempdir(DEFAULT_DOWNLOAD_PATH);
	std::string filename = tempdir + "dirListing.html";
	std::ofstream html(filename, std::ios::out);
	if (!html.is_open() || html.bad())
	{
		
		return "erroropen";
	}
	
	html << "<!DOCTYPE html>\n"
		 <<	"<html>\n"
		 << "<head>\n"
		 <<	"	<title> Directory Listing for " << "</title>\n"
		 << "</head>\n"
		 <<	"<body>\n"
		 << "<h1>\n" << " Directory Listing for " << fullPath << "</h1>\n";

	for (const auto& entry : std::filesystem::directory_iterator(fullPath))
	{
		if (debug)std::cout << entry.path().filename() << std::endl;
		html << "<p>" << entry.path().filename();
	}
	html << "</p>";
		
	html <<	"</body>\n"
		 <<	"</html>\n";
	html.close();
	return filename;
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


std::string HttpRequest::buildFullPath(void)
{
	std::string rootDir = (*_route).getRootDir();
	std::string path = _requestLine._path;
	std::string fullPath;
	
	size_t pos = path.find('?');
	if (pos != std::string::npos)
	{
		path = path.substr(0, pos);
	}
	fullPath = rootDir + path;

	if (debug)std::cout << ORANGE<<"Build fullPath: " << fullPath << std::endl;

	auto it = _requestLine._path.rbegin();
	if(*it == '/')
	{
		if (debug)std::cout << "ends with /" << std::endl;
		// are there a default files and does one of them exist?

		for(auto it : (*_route)._defaultFile)
		{
			
			if (debug)std::cout <<ORANGE<<"checking default file: "<< it << std::endl;
			std::string temp = fullPath + it;
			if(access(temp.c_str(), F_OK) == 0)// read access?
			{
				if (debug)std::cout << "default file found: "<< it << std::endl;
				fullPath += it; //extract from config. if 2 indexes are availiable check all and give first that fits?
				break;
			} 
		}
		// return fullPath; //extract from config. if 2 indexes are availiable check all and give first that fits?
	}
	if(debug)std::cout<< RESET;
	if(fullPath.find("..") != std::string::npos) //to avoid forbidden access
	{
		return "";
	}
	
	if (debug)std::cout << ORANGE<<"fullPath: " << fullPath <<RESET<< std::endl;
	return fullPath;
}


std::string HttpRequest::getRequestedFile(void)
{

	std::string fullPath = buildFullPath();
	if(debug)std::cout <<ORANGE<< "requested File " << RESET<<fullPath << std::endl;
	if(fileExists(fullPath) && !directoryExists(fullPath))
		return(fullPath);
	if(directoryExists(fullPath))
	{
		return(serveDirectory(fullPath));
	}
	return "";
}

std::string HttpRequest::readFileContent(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())
	{
		return "erroropen";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
