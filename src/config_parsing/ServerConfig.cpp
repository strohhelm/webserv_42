#include "../../include/ServerConfig.hpp"


void	ServerConfig::setPort(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"setPort Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive 'listen' line: " + std::to_string(lineNum));
	else if (all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		_port = std::stoi(context[0].str);
	else
		throw std::runtime_error("Syntax error in directive 'listen' line: " \
								+ std::to_string(lineNum) + " \"" \
								+ context[0].str + "\""
								+ " -> only numbers allowed");

}

void	ServerConfig::setServerNames(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"ServerNames Tokens:"<<std::endl;
	printConfTokens(context);
	lineNum = 0;
	for(auto it = context.begin(); it != context.end(); it++)
	{
		_serverNames.push_back(it->str);
	}
}
void	ServerConfig::setErrorPages(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"ErrorPage Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() < 2)
		throw std::runtime_error("Syntax error in directive 'error_page': Too few arguments" );
	struct stat sb;
	std::string filename = (context.end() - 1)->str;
	if (stat(filename.data(), &sb) != 0)
		throw std::runtime_error("File not found: " + filename + " " + "line: " + std::to_string((context.end() - 1)->lineNum));
	for (auto it = context.begin(); it < context.end() - 1; it++)
	{
		int key = std::stoi(it->str);
		if (StatusCode.find(key) != StatusCode.end())
		{
			if (_errorPage.find(key) != _errorPage.end())
				_errorPage.insert({key, filename});
			else
				throw std::runtime_error("Double Errorcode set: \"" + it->str + "\" line: " += std::to_string(it->lineNum));
		}
		else
			throw std::runtime_error("Unknown Errorcode: \"" + it->str + "\" line: " += std::to_string(it->lineNum));
	}
}

void	ServerConfig::setClientBodySize(std::vector<confToken>	&context, size_t lineNum)
{
	std::cout<<"ErrorPage Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive 'client_max_body_size'");
	std::smatch match;
	std::regex pattern("^\\d+[a-zA-Z]{2}$");
	if (!std::regex_match(context[0].str, match, pattern))
		throw std::runtime_error("In directive 'client_max_body_size': Not a valid Size: \"" + context[0].str + "\" line: " + std::to_string(context[0].lineNum));
	size_t pos = context[0].str.length() - 2;
	std::string unit = (context[0].str.substr(pos));
	size_t size = std::stoul(context[0].str.substr(0, pos));
	if (unit == "kb" || unit == "KB")
		size *= 1024; // 2 ^ 10 
	else if (unit == "mb" || unit == "MB")
		size *= 1048576; // 2 ^ 20
	else if (unit == "gb" || unit == "GB")
		size *= 1073741824; // 2 ^ 30
	else
		throw std::runtime_error("In directive 'client_max_body_size': Not a valid Size: \"" + context[0].str + "\" line: " + std::to_string(context[0].lineNum));
	_maxBody = size;
}
void	ServerConfig::setIndex(std::vector<confToken>	&context, size_t lineNum)
{
	std::cout<<"setIndex Tokens:"<<std::endl;
	printConfTokens(context);
	lineNum = 0;
	for(auto it = context.begin(); it != context.end(); it++)
	{
		// struct stat sb;
		// if (stat(it->str.data(), &sb) != 0)
		// 	throw std::runtime_error("File not found: " + it->str + " " + "line: " + std::to_string((it->lineNum)));
		_indexFile.push_back(it->str);
	}
}
void	ServerConfig::setRootDir(std::vector<confToken>	&context, size_t lineNum)
{
	std::cout<<"RootDir Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive 'listen' line: " + std::to_string(lineNum));
	else
	{
		// struct stat sb;
		// if (stat(context[0].str.data(), &sb) != 0)
		// 	throw std::runtime_error("In directive 'root' line: " + std::to_string(lineNum) \
		// 							+ " -> File not found: \"" \
		// 							+ context[0].str + "\"");
		// else
			_rootDir = context[0].str;
	}
}

void	ServerConfig::setRoute(std::vector<confToken>	&context, size_t lineNum)
{
	std::cout<<"Route Tokens:"<<std::endl;
	printConfTokens(context);

}

void ServerConfig::setDefaultValues(void)
{

}
ServerConfig::ServerConfig(std::vector<confToken> context)
{
	setDefaultValues();
	std::map <std::string, void(ServerConfig::*)(std::vector<confToken> &, size_t lineNum)> directives;
	directives.insert({std::string("listen"), &ServerConfig::setPort});
	directives.insert({std::string("server_name"), &ServerConfig::setServerNames});
	directives.insert({std::string("location"), &ServerConfig::setRoute});
	directives.insert({std::string("root"), &ServerConfig::setRootDir});
	directives.insert({std::string("error_pages"), &ServerConfig::setErrorPages});
	directives.insert({std::string("client_max_body_size"), &ServerConfig::setClientBodySize});
	directives.insert({std::string("index"), &ServerConfig::setIndex});
	std::cout<<"Server --------------------------------------------------------------------------------------------------------"<<std::endl;
	printConfTokens(context);
	parseTokens<ServerConfig>(context, directives, *this);
	std::cout<<"Server end ----------"<<std::endl;
}