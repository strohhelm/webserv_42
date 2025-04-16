
#ifndef SERVERCONFIG
#define SERVERCONFIG

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <regex>
#include <string>
#include "./StatusCodes.hpp"
#include <sys/stat.h>
#include <chrono>
#include "./Colors.hpp"
#define DEFAULT_CONFIG_PATH "../../config/test.conf"
#define DEFAULT_ERROR_LOG "../../logs/error.log"
#define DEFAULT_ACCESS_LOG "../../logs/access.log"

enum ConfTokenType {
	DIRECTIVE,
	VALUE,
	STOP,
	BLOCK_START,
	BLOCK_END,
};

struct confToken
{
	std::string		str;
	ConfTokenType	type;
	size_t			lineNum;
	confToken(const std::string& word, size_t line) : str(word), type(DIRECTIVE), lineNum(line) {};
};

class routeConfig
{
	bool						_methods[3]; //Define a list of accepted HTTP methods for the route
	int 						_redirectCode;
	std::string					_redirectPath; //Define a HTTP redirection
	std::string					_rootDir; //Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet).
	bool						_dirListing; //Turn on or off directory listing.
	std::vector<std::string>	_defaultFile; //Set a default file to answer if the request is a directory.
	std::string					_uploadPath; //Make the route able to accept uploaded files and configure where they should be saved.
	std::string					_cgiExtension; //nExecute CGI based o certain file extension (for example .php)

	public:
		routeConfig(std::vector<confToken> &context);
		void	printConfig(std::string path);
		void	setDefaultValues();
		void	checkValues(void);
		void	setMethods(std::vector<confToken>		&context, size_t lineNum);
		void	setRedirect(std::vector<confToken>		&context, size_t lineNum);
		void	setRootDir(std::vector<confToken>		&context, size_t lineNum);
		void	setAutoIndex(std::vector<confToken>		&context, size_t lineNum);
		void	setDefaultFiles(std::vector<confToken>	&context, size_t lineNum);
		void	setUploadPath(std::vector<confToken>	&context, size_t lineNum);
		void	setCGIExtension(std::vector<confToken>	&context, size_t lineNum);

};

/*

∗ Do you wonder what a CGI is?
∗ Because you won’t call the CGI directly, use the full path as PATH_INFO.
∗ Just remember that, for chunked request, your server needs to unchunk
it, the CGI will expect EOF as end of the body.
∗ Same things for the output of the CGI. If no content_length is returned
from the CGI, EOF will mark the end of the returned data.
∗ Your program should call the CGI with the file requested as first argument.
∗ The CGI should be run in the correct directory for relative path file access.
∗ Your server should work with one CGI (php-CGI, Python, and so forth).

*/

class ServerConfig
{
	private:
		int									_port; //default set to 80
		std::vector<std::string>			_serverNames; //default set to localhost?
		// std::unordered_map<std::string, int> urls;

		std::string							_rootDir;
		std::vector<std::string>			_indexFile; //default set to index.html?

		std::map<int, std::string>			_errorPage; // 404 ./var/www/html/40x.html
		size_t								_maxBody;
		std::map<std::string, routeConfig>	_routes; // path and config

	public:
		ServerConfig() = delete;
		ServerConfig(std::vector<confToken> context);
		~ServerConfig(){};
		void	printConfig();
		void	setDefaultValues(void);
		void	checkValues(void);
		void	setUrl(const std::vector<std::string>& serverNames ,const int& port);
		int		getPort(void);
		void	setRootDir(const std::string& rootDir);
		const	std::string& getRootDir(void);
		void	setPort(std::vector<confToken>				&context, size_t lineNum);
		void	setServerNames(std::vector<confToken>		&context, size_t lineNum);
		void	setErrorPages(std::vector<confToken>		&context, size_t lineNum);
		void	setClientBodySize(std::vector<confToken>	&context, size_t lineNum);
		void	setIndex(std::vector<confToken>				&context, size_t lineNum);
		void	setRoute(std::vector<confToken>				&context, size_t lineNum);
		void	setRootDir(std::vector<confToken>			&context, size_t lineNum);
};

class MainConfig
{
	public:
		std::pair<std::string, std::ofstream> _error_log;
		std::pair<std::string, std::ofstream> _access_log;
		size_t _worker_connections;
		size_t _keepalive_timeout;
		std::vector<ServerConfig> _http;
		void tokenizeConfig(std::vector<confToken> &tokens);
		void rmComment(std::string &line);
		void prepareLine(std::string &line, size_t lineNum);
		void typesortTokens(std::vector<confToken> &tokens);
		void setErrorLog(std::vector<confToken>		&tokens, size_t lineNum);
		void setAccessLog(std::vector<confToken>	&tokens, size_t lineNum);
		void setWorkConn(std::vector<confToken>		&tokens, size_t lineNum);
		void setTimeout(std::vector<confToken>		&tokens, size_t lineNum);
		void setHttp(std::vector<confToken>			&tokens, size_t lineNum);
		
		public:
		MainConfig(void);
		~MainConfig(){};
		MainConfig(MainConfig& src) = delete;
		MainConfig& operator=(MainConfig &src) = delete;
		void	checkValues(void);
		void	setDefaultValues(void);
		void	printConfig();

	};
	void collectContext(std::vector<confToken> &tokens, std::vector<confToken>::iterator it, std::vector<confToken> &context);
	void printConfTokens(std::vector<confToken>	&tokens);
	void OpenFile(std::pair<std::string, std::ofstream> &file);

	template <typename type> void parseTokens(std::vector<confToken> &tokens, std::map <std::string, void(type::*)(std::vector<confToken> &, size_t lineNum)> directives, type &obj);
	#include "../src/config_parsing/tokens.tpp"

#endif
