#include"../include/ServerConfig.hpp"

#include <filesystem>


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
	{
		_state._errorOcurred = 401;
		return "";
	}
	std::string tempdir(DEFAULT_TEMP_PATH);
	std::string filename = tempdir + "dirListing.html";
	std::ofstream html(filename, std::ios::out);
	if (!html.is_open() || html.bad())
	{
		_state._errorOcurred = 500;
		return "";
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
	path = path.substr(path.find((*_route)._path) + (*_route)._path.length());

	fullPath = rootDir +(path.front() == '/' ? "":"/")+ path;

	if (debug)std::cout << ORANGE<<"Build fullPath: " << fullPath << std::endl;

	auto it = fullPath.rbegin();
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
				fullPath += it;
				break;
			} 
		}
	
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
	std::ifstream file(path, std::ios::in);
	if(debug)std::cout <<ORANGE<< "Reading file content: " << RESET<<path << std::endl;
	if(!file.is_open())
	{
		_state._errorOcurred = 500;
		if(debug)std::cout <<ORANGE<<path<< "not open" << RESET<<path << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}
