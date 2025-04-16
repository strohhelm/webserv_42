#include "../../include/ServerConfig.hpp"

void MainConfig::prepareLine(std::string &line, size_t lineNum)
{
	line = std::regex_replace(line, std::regex(R"(\{)"), " {");
	line = std::regex_replace(line, std::regex(R"(\})"), " }");
	line = std::regex_replace(line, std::regex(R"(\;)"), " ;");

	std::smatch match;
	if (!std::regex_match(line, match, std::regex("^\\s*$|.*[;{}]\\s*$" )))
	{
		throw std::runtime_error("Missing delimiter in line: " + std::to_string(lineNum));
	}
}

void MainConfig::rmComment(std::string &line)
{
	size_t pos = line.find_first_of("#");
	if (pos != std::string::npos)
		line.erase(line.begin() + pos, line.end());
}

void MainConfig::tokenizeConfig(std::vector<confToken> &tokens)
{
	std::string				filename = DEFAULT_CONFIG_PATH;
	std::ifstream			file (filename, std::fstream::in);
	std::stringstream		buffer;
	std::string				line;
	std::string				word;
	size_t					lineNum = 0;

	if (! file.is_open() || file.bad())
		throw std::runtime_error("Error, couldnt open file: " + filename);
	while (getline(file, line))
	{
		lineNum++;
		rmComment(line);
		prepareLine(line, lineNum);
		buffer.clear();
		buffer<<line;
		while (buffer>>word)
			tokens.push_back(confToken(word, lineNum));
	}
	// printConfTokens(tokens);
}

void MainConfig::typesortTokens(std::vector<confToken>& tokens)
{
	int start = 0;
	for (auto &token :tokens)
	{
		if (token.str == ";")
		{
			token.type = STOP;
			start = 0;
		}
		else if (token.str == "{")
		{
			token.type = BLOCK_START;
			start = 0;
		}
		else if (token.str == "}")
		{
			token.type = BLOCK_END;
			start = 0;
		}
		else if (!start++)
			token.type = DIRECTIVE;
		else
			token.type = VALUE;
	}
}

void MainConfig::setErrorLog(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"Error_log Tokens:"<<std::endl;
	// printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive error_log line: " + std::to_string(lineNum));
	else
		_error_log.first = context[0].str;
}

void MainConfig::setAccessLog(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"Access_log  Tokens:"<<std::endl;
	// printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive access_log line:" + std::to_string(lineNum));
	else
		_access_log.first = context[0].str;
}

void MainConfig::setWorkConn(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"Worker connect Tokens:"<<std::endl;
	// printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive worker_connections line:" + std::to_string(lineNum));
	else if (!all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		throw std::runtime_error("Value error in directive worker_connections: \"" + context[0].str + "\" not a number");
	else
		_worker_connections = std::stoul(context[0].str);
}

void MainConfig::setTimeout(std::vector<confToken> &context, size_t lineNum)
{
	// std::cout<<"Timeout Tokens:"<<std::endl;
	// printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive keepalive_timeout");
	else if (!all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		throw std::runtime_error("Value error in directive keepalive_timeout: \"" + context[0].str + "\" not a number");
	else
		_keepalive_timeout = std::stoul(context[0].str);
}

void MainConfig::setHttp(std::vector<confToken> &context, size_t lineNum)
{
	if (context.begin()->type == BLOCK_START && (context.end() - 1)->type == BLOCK_END)
	{
		// std::cout<<"Http Tokens:"<<std::endl;
		// printConfTokens(context);
		for(auto it = context.begin() + 1; it < (context.end() - 1); it++)
		{
			if (!(it->type == DIRECTIVE && it->str == "server"))
			{
				throw std::runtime_error("[setHttp]: Syntax Error: \""
					+ it->str + "\"" + " line: "
					+ std::to_string(it->lineNum)
					+ " -> directive not valid!");
				}
				else
				{
					std::vector<confToken>servercontext;
					collectContext(context, it, servercontext);
					it += servercontext.size();
					servercontext.erase(servercontext.begin());
					servercontext.erase(servercontext.end() - 1);
					_http.push_back(ServerConfig(servercontext));
				}
			}
		}
		else 
			throw std::runtime_error("WTF that shouldnt happen at all");
}


void MainConfig::setDefaultValues(void)
{
	_error_log.first = DEFAULT_ERROR_LOG;
	_access_log.first = DEFAULT_ACCESS_LOG;
	_worker_connections = 10;
	_keepalive_timeout = 75;
	_http.clear();
}

void MainConfig::checkValues(void)
{
	OpenFile(_error_log);
	OpenFile(_access_log);
	if (_worker_connections < 1)
		throw std::runtime_error("Value worker_connections in main context not valid, must be bigger than 1.");
	if (! _http.size())
		throw std::runtime_error("Must have servers!!!");
	for (auto server:_http)
		server.checkValues();
}

void MainConfig::printConfig(void)
{
	std::stringstream print;
	print<<MAGENTA<<UNDERLINE<<"MAIN CONFIG:"<<RESET<<"\n";
	print<<"Error Log: "<<BLUE<<_error_log.first<<RESET<<"\n";
	print<<"Access Log: "<<BLUE<<_access_log.first<<RESET<<"\n";
	print<<"Worker Connections: "<<BLUE<<_worker_connections<<RESET<<"\n";
	print<<"Keepalive Timeout: "<<BLUE<<_keepalive_timeout<<RESET<<"\n";
	std::cout<<print.str()<<std::endl;
	for (auto i: _http)
		i.printConfig();
}

MainConfig::MainConfig(void)
{
	setDefaultValues();

	std::map <std::string, void(MainConfig::*)(std::vector<confToken> &, size_t lineNum)> directives;
	directives.insert({std::string("error_log"), &MainConfig::setErrorLog});
	directives.insert({std::string("access_log"), &MainConfig::setAccessLog});
	directives.insert({std::string("worker_connections"), &MainConfig::setWorkConn});
	directives.insert({std::string("keepalive_timeout"), &MainConfig::setTimeout});
	directives.insert({std::string("http"), &MainConfig::setHttp});

	std::vector<confToken> tokens;
	tokenizeConfig(tokens);
	typesortTokens(tokens);
	// std::cout<<"All Tokens:"<<std::endl;
	// printConfTokens(tokens);
	parseTokens<MainConfig>(tokens, directives, *this);
	checkValues();
	printConfig();
}


