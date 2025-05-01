#include"../include/ServerConfig.hpp"

bool HttpRequest::validateHost(std::vector<std::string> &serverNames)
{
	std::string host = _headers["Host"];
	host = host.substr(0, host.find_first_of(':'));
	if (debug)std::cout<<GREEN<<"HOST: "<<YELLOW<<host<<std::endl;
	if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end() || host == "127.0.0.1" || (host.find("10.1") == 0) )
		return true;
	else
		return false;
}

int	HttpRequest::checkCgi(std::string path)
{
	// std::string filename = path.substr(path.find_last_of("/"), path.size() - path.find_last_of("/"));
	// bool check = (filename.substr(filename.size() - 4, filename.size()) == ".php");


	std::filesystem::path filepath(path.substr(0, path.find('?')));
	
	int iscgi = (*_route).checkCgiPath(filepath.extension());
	if (debug)std::cout << ORANGE<<"is CGI " << filepath.extension().string()<<" "<<MAGENTA<< (iscgi == 1 ? "true" : "false") << RESET<< std::endl;
	return iscgi;
}

int HttpRequest::validateRequest(void)
{
	if (debug)std::cout<<ORANGE<<"Validating request"<<RESET<<std::endl;
	std::string path = _requestLine._path;
	auto &routes = (*_config)._routes;
	ServerConfig* conf = _config;
	if (!conf)
		return 500;
	if (!validateHost((*_config)._serverNames))
	{
		if (debug)std::cout << BG_BRIGHT_MAGENTA << "Host invalid" << RESET << std::endl;
		return 400;
	}
	// does route exist?
	while (!path.empty())
	{
		auto tmp = routes.find(path);
		if (tmp != routes.end())
		{
			_route = &(tmp->second);
			return 0;
		}
		else
		{
			if (path.find_last_of('/') == path.find_first_of('/'))
			{
				auto it = (*_config)._routes.find("/");
				if ( it != (*_config)._routes.end())
				{
					_route = &(it->second);
					return 0;
				}
			}
			size_t pos = path.find_last_of('/');
			if (pos == std::string::npos)
			break;
			path = path.substr(0, pos);
		}
	}	
	return 404;
}