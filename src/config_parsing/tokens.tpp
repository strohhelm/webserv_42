template <typename type>
void parseTokens(std::vector<confToken>	&tokens, std::map <std::string, void(type::*)(std::vector<confToken> &, size_t lineNum)> directives, type &obj)
{
	for (auto it = tokens.begin(); it < tokens.end(); it++)
	{
		if (it->type == DIRECTIVE)
		{
			auto dir = directives.find(it->str);
			if (dir != directives.end())
			{
				std::vector<confToken> context;
				collectContext(tokens, it, context);
				(obj.*directives[it->str])(context, it->lineNum);
				it += context.size() + 1;
			}
			else{
				throw std::runtime_error("[parseTokens]: Unknown directive in config file: \""
									+ it->str + "\"" + " line: "
									+ std::to_string(it->lineNum));
			}
		}
		else
		{
			throw std::runtime_error("[parseTokens]: Unknown token in config file: \""
									+ it->str + "\"" + " line: "
									+ std::to_string(it->lineNum));
		}
	}
}
