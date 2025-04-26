#include "../include/ServerConfig.hpp"


void	routeConfig::setMethods(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"Methods Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	if (context.size() > 3)
		throw std::runtime_error("[setMethods]: Too many arguments to directive 'methods' line: " + line);
	else if ( context.size() < 1)
		throw std::runtime_error("[setMethods]: No arguments to directive 'methods' line: " + line);
	else
	{
		_methods[0] = false; //default is set true
		for (auto it = context.begin(); it != context.end(); it++)
		{
			if (it->str == "GET")
				_methods[static_cast<size_t>(HttpMethod::GET)] = true; //Could have done 0, 1, 2 for indeces, but wanted to make sure and readable its the correct index
			else if (it->str == "POST")
				_methods[static_cast<size_t>(HttpMethod::POST)] = true;
			else if (it->str == "DELETE")
				_methods[static_cast<size_t>(HttpMethod::DELETE)] = true;
			else
				throw std::runtime_error("[setMethods]: Unkown method: \"" + it->str + "\" line: " + line);
		}
	}
}
void	routeConfig::setRedirect(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"RedirectCode Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	if (context.size() != 2)
		throw std::runtime_error("Syntax error in directive 'redirect' line: " + line);
	if (all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
	{
		if (context[0].str[0] == '3')
		{
			int code = std::stoi(context[0].str);
			if (StatusCode.find(code) != StatusCode.end())
			{
				_redirectCode = code;
			}
			else
				throw std::runtime_error("Status code \"" + context[0].str + "\" not valid. line" + line);
		}
		else
			throw std::runtime_error("Status code \"" + context[0].str + "\" must begin with '3'. line" + line);
	}
	else
		throw std::runtime_error("Status code \"" + context[0].str + "\" not numeric. line" + line);
	_redirectPath = context[1].str; //Error handling here? My gut says no , better in handling logic.
}

void	routeConfig::setRootDir(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"RootDir Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	if (context.size() != 1)
		throw std::runtime_error("Syntax error in directive 'root' line: " + line);
	_rootDir = context[0].str;
}
void	routeConfig::setAutoIndex(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"AutoIndex Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive 'redirect' line: " + line);
	if (context[0].str == "on" || context[0].str == "ON")
		_dirListing = true;
	else if (context[0].str == "off" || context[0].str == "OFF")
		_dirListing =false;
	else
		throw std::runtime_error(" in directive 'autoindex' line: " + line + "unknown option: \"" + context[0].str +"\"");
}
void	routeConfig::setDefaultFiles(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"DefaultFile Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	for (auto t:context)
	{
		if (std::find(_defaultFile.begin(), _defaultFile.end(), t.str) != _defaultFile.end() || _defaultFile.empty())
			_defaultFile.push_back(t.str);
	}
	if (!context.size())
		throw std::runtime_error("directive 'index' set but no arguments given. line: "+ std::to_string(lineNum));
}

void	routeConfig::setUploadPath(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"UploadPath Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	if (context.size() != 1)
		throw std::runtime_error("Syntax error in directive 'upload_path' line: " + line);
	_uploadPath = context[0].str;

}
void	routeConfig::setCGIExtension(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"CGI Tokens:"<<std::endl;
	// printConfTokens(context);
	std::string line = std::to_string(lineNum);
	if (context.size() != 2)
		throw std::runtime_error("Syntax error in directive 'cgi' line: " + line + " Usage: [ cgi <extension> <path_to_executable> ]");
	_cgiExtension[context[0].str] = context[1].str;
}

void routeConfig::setDefaultValues()
{
	_methods[0] = true; _methods[1] = false; _methods[2] = false;
	_redirectCode = 0;
	_redirectPath.clear();
	_rootDir.clear();
	_dirListing = false;
	_uploadPath.clear();
	_errorcode = routeError::All_GOOD;
	_defaultFile.clear();
}

void	routeConfig::checkValues(ServerConfig& conf)
 {
	if (_rootDir.empty())
		_rootDir = conf._rootDir;
	if (_defaultFile.empty())
		_defaultFile = conf._indexFile;
}

bool	routeConfig::checkMethod(HttpMethod& method)
{
	return(_methods[static_cast<int>(method)]);
}

int	routeConfig::checkCgiPath(std::string fileextension)
{
	if (_cgiExtension.find(fileextension) != _cgiExtension.end())
	{
		if (!access(_cgiExtension[fileextension].c_str(), F_OK) && !access(_cgiExtension[fileextension].c_str(), X_OK))
			return 1;
		else
			return -1;
	}
	return 0;
}

std::string& routeConfig::getCgiPath(std::string fileextension)
{
	return _cgiExtension[fileextension];
}

void routeConfig::printConfig(std::string path)
{
	std::stringstream print;
	print<<ORANGE<<UNDERLINE<<"ROUTE CONFIG:"<< GREEN<<"\""<<path<<"\""<< RESET<<"\n";
	print<<"Methods: "<<BLUE;if (_methods[0]) print<<"GET ";if(_methods[1])print<<"POST ";if(_methods[2])print<<"DELETE ";print<<RESET<<"\n";
	print<<"Redirect Code: "<<BLUE<<_redirectCode<<RESET<<"\n";
	print<<"Redirect Path: "<<BLUE<<_redirectPath<<RESET<<"\n";
	print<<"Root Dir: "<<BLUE<<_rootDir<<RESET<<"\n";
	print<<"Autoindex: "<<BLUE<<(_dirListing ? "ON" : "OFF")<<RESET<<"\n";
	print<<"Default File: "<<BLUE;
	for (std::string i: _defaultFile)
		print<<i<<" ";
	print<<RESET<<"\n";
	print<<"Upload Path: "<<BLUE<<_uploadPath<<RESET<<"\n";
	print<<"CGI : "<<BLUE<<RESET<<"\n";
	for (auto i: _cgiExtension)
		print<<"[ "<<i.first<<" -> "<< i.second<<" ]";
	std::cout<<print.str()<<std::endl;
}
routeConfig::routeConfig(void)
{
	_errorcode = routeError::INVALID;
}

routeConfig::routeConfig(std::vector<confToken> &context)
{
	setDefaultValues();
	std::map <std::string, void(routeConfig::*)(std::vector<confToken> &, size_t lineNum)> directives;
	directives.insert({std::string("methods"),		&routeConfig::setMethods});
	directives.insert({std::string("return"),		&routeConfig::setRedirect});
	directives.insert({std::string("root"),			&routeConfig::setRootDir});
	directives.insert({std::string("autoindex"),	&routeConfig::setAutoIndex});
	directives.insert({std::string("index"),		&routeConfig::setDefaultFiles});
	directives.insert({std::string("upload_path"),	&routeConfig::setUploadPath});
	directives.insert({std::string("cgi"),			&routeConfig::setCGIExtension});
	// std::cout<<" ------------------------------------------------Location--------------------------------------------------------"<<std::endl;
	// std::cout<<"Route Tokens:"<<std::endl;
	// printConfTokens(context);
	parseTokens<routeConfig>(context, directives, *this);
	// std::cout<<"------Location end ----"<<std::endl;
}