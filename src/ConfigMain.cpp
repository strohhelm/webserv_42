#include "../include/ServerConfig.hpp"

void MainConfig::setWorkConn(std::vector<confToken> &context, size_t lineNum)
{
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive 'worker_connections' line:" + std::to_string(lineNum));
	else if (!all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		throw std::runtime_error("Value error in directive 'worker_connections': \"" + context[0].str + "\" not a number");
	else
		_worker_connections = std::stoul(context[0].str);
}

void MainConfig::setTimeout(std::vector<confToken> &context, size_t lineNum)
{
	if (context.size() != 1 || context[0].type != VALUE)
		throw std::runtime_error("Syntax error in directive 'keepalive_timeout' line: " + std::to_string(lineNum));
	else if (!all_of(context[0].str.begin(), context[0].str.end(), [](char c){return std::isdigit(c);}))
		throw std::runtime_error("Value error in directive 'keepalive_timeout': \"" + context[0].str + "\" not a number");
	else
	{
		try
		{
			_keepalive_timeout = std::stoul(context[0].str);
		}
		catch(std::exception& e){throw std::runtime_error("Shit went down in stoul() keepalive_timeout context.");}
	}
}

void MainConfig::setHttp(std::vector<confToken> &context, size_t lineNum)
{
	if (!_http.empty())
		throw std::runtime_error("Only one \"http\" directive allowed! line:" + std::to_string(lineNum));
	if (context.begin()->type == BLOCK_START && (context.end() - 1)->type == BLOCK_END)
	{
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
	if (_http.empty())
		throw std::runtime_error("Empty \"http\" directive! line:" + std::to_string(lineNum));
}

void MainConfig::setDefaultValues(void)
{
	_worker_connections = DEFAULT_MAX_WORKER_CONNECTIONS;
	_keepalive_timeout = DEFAULT_TIMEOUT;
	_http.clear();
}

void MainConfig::checkValues(void)
{
	if (_worker_connections < 1)
		throw std::runtime_error("Value worker_connections in main context not valid, must be bigger than 1.");
	else if (_worker_connections >= 10240)
		throw std::runtime_error("Value worker_connections in main context not valid, must be smaller than 10240.");
	if (! _http.size())
		throw std::runtime_error("Must have servers!!!");
	for (auto& server:_http)
		server.checkValues();
}

void MainConfig::printConfig(void)
{
	std::stringstream print;
	print<<MAGENTA<<UNDERLINE<<"MAIN CONFIG:"<<RESET<<"\n";
	print<<"Worker Connections: "<<BLUE<<_worker_connections<<RESET<<"\n";
	print<<"Keepalive Timeout: "<<BLUE<<_keepalive_timeout<<RESET<<"\n";
	std::cout<<print.str()<<std::endl;
	for (auto &i: _http)
		i.printConfig();
}

MainConfig::MainConfig(std::string &filename)
{
	setDefaultValues();

	std::map <std::string, void(MainConfig::*)(std::vector<confToken> &, size_t lineNum)> directives;
	directives.insert({std::string("worker_connections"), &MainConfig::setWorkConn});
	directives.insert({std::string("keepalive_timeout"), &MainConfig::setTimeout});
	directives.insert({std::string("http"), &MainConfig::setHttp});

	std::vector<confToken> tokens;
	tokenizeConfig(tokens, filename);
	typesortTokens(tokens);
	// std::cout<<"All Tokens:"<<std::endl;
	// printConfTokens(tokens);
	parseTokens<MainConfig>(tokens, directives, *this);
	checkValues();
	if (debug)printConfig();
}


