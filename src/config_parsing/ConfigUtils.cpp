#include "../../include/HttpRequest.hpp"


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

void OpenFile(std::pair<std::string, std::ofstream> &file)
{
	std::ofstream& ofile = file.second;
	std::string& path = file.first;
	
	struct stat sb;
	int err = stat(path.data(), &sb);

	if (S_ISDIR(sb.st_mode))
		throw std::runtime_error("[OpenFile]: \"" + path + "\" is a directory");
	ofile.open(path, std::ios::app);
	if (!ofile.is_open() || ofile.bad())
		throw std::runtime_error("[OpenFile]: Failed to create file \"" + path + "\"");
	std::chrono::duration<int,std::ratio<60*60*24> > one_day;
	std::chrono::system_clock::time_point today = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t ( today );
	ofile<<"SESSION STARTED -- " << ctime(&tt);
}