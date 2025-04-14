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
void printConfTokens(std::vector<confToken>	&tokens)
{
	size_t tmp = 0;
	std::string lol[] = {"DIRECTIVE", "VALUE", "STOP", "BLOCK_START", "BLOCK_END"};
	int f = 0;
	// std::cout<<"\n";
	for (auto p :tokens)
	{	if (tmp < p.lineNum)
		{
			if (f > 0)
				std::cout<<std::endl;
			else
				f = 1;
			std::cout<<p.lineNum<<" ";
			tmp = p.lineNum;
		}
		else
			std::cout <<" | ";
		std::cout<<lol[p.type]<<"("<<p.str<<")";
	}
	std::cout<<"\n---------"<<std::endl;
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
	std::cout<<"Error_log Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive error_log line: " + std::to_string(lineNum));
	else
		error_log = context[0].str;
}


void MainConfig::setAccessLog(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"Access_log  Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive access_log line:" + std::to_string(lineNum));
	else
		access_log = context[0].str;
}


void MainConfig::setWorkConn(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"Worker connect Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive worker_connections line:" + std::to_string(lineNum));
	else if (!all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		throw std::runtime_error("Value error in directive worker_connections: \"" + context[0].str + "\" not a number");
	else
		worker_connections = std::stoul(context[0].str);
}


void MainConfig::setTimeout(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"Timeout Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive keepalive_timeout");
	else if (!all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		throw std::runtime_error("Value error in directive keepalive_timeout: \"" + context[0].str + "\" not a number");
	else
		keepalive_timeout = std::stoul(context[0].str);
}


void MainConfig::setHttp(std::vector<confToken> &context, size_t lineNum)
{
	if (context.begin()->type == BLOCK_START && (context.end() - 1)->type == BLOCK_END)
	{
		std::cout<<"Http Tokens:"<<std::endl;
		printConfTokens(context);
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
					servercontext.erase(servercontext.begin());
					http.push_back(ServerConfig(servercontext));
					
					it += servercontext.size() + 1;
				}
			}
		}
		else 
			throw std::runtime_error("WTF that shouldnt happen at all");
}

void collectContext(std::vector<confToken> &tokens, std::vector<confToken>::iterator it, std::vector<confToken> &context)
{
	auto	stopIt = it + 1;
	int	blocks = -1;
	while (stopIt != tokens.end())
	{
		if (stopIt->type == BLOCK_START)
		{
			if (blocks < 0)
				blocks = 0;
			blocks++;
		}
		else if (stopIt->type == BLOCK_END)
				blocks--;
		if (!blocks || (stopIt->type == STOP && blocks < 0))
		{
			if (!blocks)
				stopIt++;
			for (auto i = it + 1; i != stopIt; i++)
				context.push_back(*i);
			return;
		}
		stopIt++;
	}
	throw std::runtime_error("Unexpected End of Tokens: \""
							+ (stopIt-1)->str + "\"" + " line: "
							+ std::to_string((stopIt-1)->lineNum)
							+ " -> unclosed context for directive: \""
							+ it->str + "\"" + " line: "
							+ std::to_string((it-1)->lineNum));
}


void MainConfig::checkValues(void)
{
	
}



 MainConfig::MainConfig(void):	error_log(DEFAULT_ERROR_LOG),
							access_log(DEFAULT_ACCESS_LOG),
							worker_connections(10),
							keepalive_timeout(75)
{
	std::map <std::string, void(MainConfig::*)(std::vector<confToken> &, size_t lineNum)> directives;
	directives.insert({std::string("error_log"), &MainConfig::setErrorLog});
	directives.insert({std::string("access_log"), &MainConfig::setAccessLog});
	directives.insert({std::string("worker_connections"), &MainConfig::setWorkConn});
	directives.insert({std::string("keepalive_timeout"), &MainConfig::setTimeout});
	directives.insert({std::string("http"), &MainConfig::setHttp});

	std::vector<confToken> tokens;
	tokenizeConfig(tokens);
	typesortTokens(tokens);
	std::cout<<"All Tokens:"<<std::endl;
	printConfTokens(tokens);
	parseTokens<MainConfig>(tokens, directives, *this);
	checkValues();
}

int main (void)
{
	try{
		MainConfig config;

	}
	catch (std::exception &e)
	{
		std::cout<<"ERROR: "<<e.what()<<std::endl;
	}	// config.checkValues();
	return 0;
}
