
#ifndef CGI_HPP
# define CGI_HPP



#include <unistd.h>
#include <stdio.h>
#include <array>
#include <string>
#include <vector>
// #include"HttpRequest.hpp"
// #include "ServerConfig.hpp"


class ServerConfig;
class routeConfig;
class HttpRequest;

#define READ_FD 0
#define WRITE_FD 1

class CGI
{
	private:
		std::array<int, 2>			_parent;
		std::array<int, 2>			_child;
		char*						_argv[3];

		std::string					_phpCgiPathStr;
		const char*					_phpCgiPath;

		std::string					_scriptPath;
		std::string					_queryString;
		std::string					_fullPath;
		std::string					_requestPath;
		// HttpRequest			_request;
		int							_client_fd;
		ServerConfig				*_config;

		std::vector<std::string>	_envStrings;
		std::vector<char*>			_envp;

	public:
		void	closePipesFromFd(std::array<int, 2>& fd);
		void	closeAllPipes(void);
		int		createPipes(void);
		void	handleChildProcess(std::string method, std::string rawBody);
		void	handleParentProcess(std::string method, std::string rawBody);
		void	execute(std::string method, std::string rawBody);

		void	setPipeToRead(int fd);
		void	setPipeToWrite(int fd);
		void	setArgv(void);

		// CGI(); //TODO
		// ~CGI(); //TODO


		void setCgiParameter(const int& client_fd, ServerConfig& config, std::string& requestPath, std::string& cgiPath);
		void tokenizePath(void);
		void buildEnvStrings(std::string method, std::string rawBody);
		void convertEnvStringsToChar(void);
};


#endif