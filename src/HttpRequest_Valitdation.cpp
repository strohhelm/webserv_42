#include"../include/ServerConfig.hpp"

bool HttpRequest::validateHost(std::vector<std::string> &serverNames)
{
	std::string host = _headers["Host"];
	host = host.substr(0, host.find(':'));
	if (debug)std::cout<<GREEN<<"HOST: "<<YELLOW<<host<<std::endl;
	if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end())
		return true;
	else
		return false;
}

int	HttpRequest::checkCgi(std::string path, routeConfig& route)
{
	// std::string filename = path.substr(path.find_last_of("/"), path.size() - path.find_last_of("/"));
	// bool check = (filename.substr(filename.size() - 4, filename.size()) == ".php");

	bool endsWithPhp = path.size() >= 4 && path.substr(path.size() - 4) == ".php";

	if (debug)std::cout << ORANGE<<"ends with .php " << (endsWithPhp == true ? "true" : "false") <<RESET<< std::endl;
	if(!endsWithPhp)
		return 0;

	if (endsWithPhp && route.checkCgiPath())
		return 1;
	else 
		return -1;

}

int HttpRequest::validateRequest(ServerConfig& config, routeConfig& route)
{
	if (debug)std::cout<<ORANGE<<"Validating request"<<RESET<<std::endl;
	std::string path = _requestLine._path;
	auto routes = config._routes;
	if (!validateHost(config._serverNames))
	{
		if (debug)std::cout << BG_BRIGHT_MAGENTA << "Host invalid" << RESET << std::endl;
		return -1;
	}
	// does route exist?
	while (!path.empty())
	{
		auto tmp = routes.find(path);
		if (tmp != routes.end())
		{
			route = tmp->second;
			if (debug)std::cout << BG_BRIGHT_MAGENTA << "Request Valid" << RESET << std::endl;
			return 0;
		}
		else
		{
			if (path.find_last_of('/') == path.find_first_of('/'))
			{
				auto it = config._routes.find("/");
				if ( it != config._routes.end())
				{
					route = it->second;
					if (debug)std::cout << BG_BRIGHT_MAGENTA << "Request Valid" << RESET << std::endl;
					return 0;
				}
			}
			size_t pos = path.find_last_of('/');
			if (pos == std::string::npos)
			break;
			path = path.substr(0, pos);
		}
	}
	if (debug)std::cout << BG_BRIGHT_MAGENTA << "Request invalid" << RESET << std::endl;
	return 1;
}