#include "../include/SimpleServer.hpp"

void prepareLine(std::string &line, size_t lineNum)
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

void rmComment(std::string &line)
{
	size_t pos = line.find_first_of("#");
	if (pos != std::string::npos)
		line.erase(line.begin() + pos, line.end());
}

void tokenizeConfig(std::vector<confToken> &tokens, std::string &filename)
{
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
void typesortTokens(std::vector<confToken>& tokens)
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

// int main (void)
// {
// 	try{
// 		MainConfig config;

// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cout<<"ERROR: "<<e.what()<<std::endl;
// 	}	// config.checkValues();
// 	return 0;
// }

void OpenLogFile(std::string path, std::ofstream &ofile)
{
	struct stat sb;
	(void)stat(path.data(), &sb);

	if (S_ISDIR(sb.st_mode))
		throw std::runtime_error("[OpenLogFile]: \"" + path + "\" is a directory");
	ofile.open(path, std::ios::app);
	if (!ofile.is_open() || ofile.bad())
		throw std::runtime_error("[OpenLogFile]: Failed to create file \"" + path + "\"");
}