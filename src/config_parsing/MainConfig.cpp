#include "../../include/ServerConfig.hpp"

void MainConfig::prepareLine(std::string &line)
{
	line = std::regex_replace(line, std::regex(R"(\{)"), " {");
	line = std::regex_replace(line, std::regex(R"(\})"), " }");
	line = std::regex_replace(line, std::regex(R"(\;)"), " ;");
}

void MainConfig::rmComment(std::string &line)
{
	size_t pos = line.find_first_of("#");
	if (pos != std::string::npos)
		line.erase(line.begin() + pos, line.end());
}
void MainConfig::printConfTokens(std::vector<confToken>	&tokens)
{
	size_t tmp = 0;
	std::string lol[] = {"DIRECTIVE", "VALUE", "STOP", "BLOCK_START", "BLOCK_END"};
	for (auto p :tokens)
	{	if (tmp < p.lineNum)
		{
			std::cout<<std::endl<<p.lineNum<<" ";
			tmp = p.lineNum;
		}
		else
			std::cout <<" | ";
		std::cout<<lol[p.type]<<"("<<p.str<<")";
	}
	std::cout<<std::endl;
}

void MainConfig::tokenizeConfig(std::vector<confToken> &tokens)
{
	std::string				filename = DEFAULT_CONFIG_PATH ;
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
		prepareLine(line);
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

void MainConfig::setErrorLog(std::vector<confToken> &context)
{
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive error_log");
	else
		error_log = context[0].str;
}
void MainConfig::setAccessLog(std::vector<confToken> &context)
{
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive access_log");
	else
		access_log = context[0].str;
}
void MainConfig::setWorkConn(std::vector<confToken> &context)
{
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive worker_connections");
	else if (!all_of(context[0].str.begin(), context[0].str.end(), &isnumber))
		throw std::runtime_error("Value error in directive worker_connections: \"" + context[0].str + "\" not a number");
	else
		worker_connections = std::stoul(context[0].str);
}
void MainConfig::setTimeout(std::vector<confToken> &context)
{
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive keepalive_timeout");
	else if (!all_of(context[0].str.begin(), context[0].str.end(), &isnumber))
		throw std::runtime_error("Value error in directive keepalive_timeout: \"" + context[0].str + "\" not a number");
	else
		keepalive_timeout = std::stoul(context[0].str);
}
void MainConfig::setHttp(std::vector<confToken> &context)
{
	for(auto it = context.begin(); it != context.end(); it++)
	{
		if (!(it->type == DIRECTIVE && it->str == "server"))
		{
			throw std::runtime_error("Syntax Error: \""
				+ it->str + "\"" + " line: "
				+ std::to_string(it->lineNum)
				+ " -> directive not valid!\"");
		}
		else
		{
			std::vector<confToken>servercontext;
			collectContext(context, it, servercontext);
			// http.push_back(ServerConfig(servercontext));
		}
	}
}

void MainConfig::collectContext(std::vector<confToken> &tokens, std::vector<confToken>::iterator &it, std::vector<confToken> &context)
{
	auto	stopIt = it + 1;
	size_t	blocks = 0;
	while (stopIt != tokens.end())
	{
		if (!blocks && (stopIt->type == STOP || stopIt->type == BLOCK_END))
		{
			for (auto i = it + 1; i != stopIt; i++)
				context.push_back(*i);
			it = stopIt + 1;
			return;
		}
		else if (stopIt->type == BLOCK_END)
			blocks--;
		else if (stopIt->type == BLOCK_START)
			blocks++;
		stopIt++;
	}
	throw std::runtime_error("Unexpected EOF in config file: \""
							+ (stopIt-1)->str + "\"" + " line: "
							+ std::to_string((stopIt-1)->lineNum)
							+ " -> unclosed context for directive: \""
							+ it->str + "\"" + " line: "
							+ std::to_string((it-1)->lineNum));
}
void MainConfig::checkValues(void)
{
	
}
void MainConfig::parseTokens(std::vector<confToken>	&tokens, std::string directives[], void(MainConfig::*funcs[])(std::vector<confToken> &tokens))
{
	for (auto it = tokens.begin(); it != tokens.end(); it++)
	{
		if (it->type == DIRECTIVE)
		{
			auto dir = std::find(std::begin(*directives), std::end(*directives), it->str);
			if ( dir != std::end(*directives))
			{
				std::vector<confToken> context;
				size_t index = dir - std::begin(*directives);
				collectContext(tokens, it, context);
				(this->*funcs[index])(context);
			}
			else
			{
				throw std::runtime_error("Unknown directive in config file: \""
									+ it->str + "\"" + " line: "
									+ std::to_string(it->lineNum));
			}
		}
		else
		{
			throw std::runtime_error("Unknown token in config file: \""
									+ it->str + "\"" + " line: "
									+ std::to_string(it->lineNum));
		}
	}
}

 MainConfig::MainConfig():	error_log(DEFAULT_ERROR_LOG),
							access_log(DEFAULT_ACCESS_LOG),
							worker_connections(10),
							keepalive_timeout(75)
{
	std::string directives[]{"error_log", "access_log", "worker_connections", "keepalive_timeout", "http"};
	void(MainConfig::*funcs[])(std::vector<confToken> &tokens) = {
		&MainConfig::setErrorLog,
		&MainConfig::setAccessLog,
		&MainConfig::setWorkConn,
		&MainConfig::setTimeout,
		&MainConfig::setHttp,
	};
	if (std::size(funcs) != std::size(directives))
		throw std::runtime_error("Directives not configured correctly! Get your Code straight Philipp!");
	std::vector<confToken> tokens;
	tokenizeConfig(tokens);
	typesortTokens(tokens);
	printConfTokens(tokens);
	// parseTokens(tokens, directives, funcs);
	// checkValues();
}
int main (void)
{
	MainConfig config;
	config.checkValues();
	return 0;
}
