#include "../../include/ServerConfig.hpp"
#include "../../include/HttpRequest.hpp"

void	routeConfig::setMethods(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"Methods Tokens:"<<std::endl;
	printConfTokens(context);
	if (context.size() > 3)
		throw std::runtime_error("[setMethods]: Too many arguments to directive 'methods' line: " + std::to_string(lineNum));
	else if ( context.size() < 1)
		throw std::runtime_error("[setMethods]: No arguments to directive 'methods' line: " + std::to_string(lineNum));
	else
	{
		for (auto it = context.begin(); it != context.end(); it++)
		{
			if (it->str == "GET")
				_methods[static_cast<size_t>(HttpMethod::GET)] = true; //Could have done 0, 1, 2 for indeces, but wanted to make sure and readable its the correct index
			else if (it->str == "POST")
				_methods[static_cast<size_t>(HttpMethod::POST)] = true;
			else if (it->str == "DELETE")
				_methods[static_cast<size_t>(HttpMethod::DELETE)] = true;
			else
				throw std::runtime_error("[setMethods]: Unkown method: \"" + it->str + "\" line: " + std::to_string(lineNum));
		}
	}
}
void	routeConfig::setRedirectCode(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"RedirectCode Tokens:"<<std::endl;
	printConfTokens(context);

}
void	routeConfig::setRedirectPath(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"RedirectPath Tokens:"<<std::endl;
	printConfTokens(context);

}
void	routeConfig::setRootDir(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"RootDir Tokens:"<<std::endl;
	printConfTokens(context);

}
void	routeConfig::setDirListing(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"DirListing Tokens:"<<std::endl;
	printConfTokens(context);

}
void	routeConfig::setDefaultFiles(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"DefaultFile Tokens:"<<std::endl;
	printConfTokens(context);

}
void	routeConfig::setUploadPath(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"UploadPath Tokens:"<<std::endl;
	printConfTokens(context);

}
void	routeConfig::setCGIExtension(std::vector<confToken> &context, size_t lineNum)
{
	std::cout<<"CGI Tokens:"<<std::endl;
	printConfTokens(context);

}
void routeConfig::setDefaultValues()
{

}

routeConfig::routeConfig(std::vector<confToken> &context)
{
	setDefaultValues();
	std::map <std::string, void(routeConfig::*)(std::vector<confToken> &, size_t lineNum)> directives;
	directives.insert({std::string("methods"), &routeConfig::setMethods});
	directives.insert({std::string("redirect"), &routeConfig::setRedirectCode});
	directives.insert({std::string("redirect_path"), &routeConfig::setRedirectPath});
	directives.insert({std::string("root"), &routeConfig::setRootDir});
	directives.insert({std::string("directory_listing"), &routeConfig::setDirListing});
	directives.insert({std::string("index"), &routeConfig::setDefaultFiles});
	directives.insert({std::string("upload_path"), &routeConfig::setUploadPath});
	directives.insert({std::string("cgi"), &routeConfig::setCGIExtension});
	std::cout<<"Location --------------------------------------------------------------------------------------------------------"<<std::endl;
	std::cout<<"Route Tokens:"<<std::endl;
	printConfTokens(context);
	parseTokens<routeConfig>(context, directives, *this);
	std::cout<<"Location end ----------"<<std::endl;

}