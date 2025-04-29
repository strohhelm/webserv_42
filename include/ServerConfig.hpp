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
#include "./HttpRequest.hpp"
#include "./Macros.hpp"
#include <filesystem>

enum class HttpMethod;

enum ConfTokenType {
	DIRECTIVE,
	VALUE,
	STOP,
	BLOCK_START,
	BLOCK_END,
};

enum class routeError
{
	All_GOOD,
	NO_SERVERNAME,
	INVALID
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
	public:

	std::string					_path;
	bool						_methods[3]; //Define a list of accepted HTTP methods for the route
	std::pair<int, std::string>	_redirect; //Define a HTTP redirection
	std::string					_rootDir; //Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet).
	bool						_dirListing; //Turn on or off directory listing.
	std::vector<std::string>	_defaultFile; //Set a defcclean
	std::string					_uploadPath; //Make the route able to accept uploaded files and configure where they should be saved.
	std::string					_downloadPath; //Make the route able to accept uploaded files and configure where they should be saved.
	std::map<std::string, std::string>	_cgiExtension; //nExecute CGI based o certain file extension (for example .php)
	routeError					_errorcode;
	
	routeConfig(std::string& path, std::vector<confToken> &context);
	routeConfig();
	void	printConfig(std::string path);
	void	setDefaultValues(std::string& path);
	void	checkValues(ServerConfig& conf);
	bool	checkMethod(HttpMethod& method);
	int		checkCgiPath(std::string fileextension);
	std::string& getCgiPath(std::string fileextension);
	void	setMethods(std::vector<confToken>		&context, size_t lineNum);
	void	setRedirect(std::vector<confToken>		&context, size_t lineNum);
	void	setRootDir(std::vector<confToken>		&context, size_t lineNum);
	void	setAutoIndex(std::vector<confToken>		&context, size_t lineNum);
	void	setDefaultFiles(std::vector<confToken>	&context, size_t lineNum);
	void	setUploadPath(std::vector<confToken>	&context, size_t lineNum);
	void	setDownloadPath(std::vector<confToken>	&context, size_t lineNum);
	void	setCGIExtension(std::vector<confToken>	&context, size_t lineNum);
	bool	isDirListingActive(void);
	std::string getRootDir(void);

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
	public:
		int									_port; //default set to 80
		std::vector<std::string>			_serverNames; //default set to localhost?
		// std::unordered_map<std::string, int> urls;

		std::string							_rootDir;
		std::vector<std::string>			_indexFile; //default set to index.html?

		std::map<int, std::string>			_errorPage; // 404 ./var/www/html/40x.html
		size_t								_maxBody;
		std::map<std::string, routeConfig>	_routes; // path and config


		ServerConfig(){};
		ServerConfig(std::vector<confToken> context);
		~ServerConfig(){};
		void	printConfig();
		void	setDefaultValues(void);
		void	checkValues(void);

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
		std::string _error_log;
		std::string _access_log;
		size_t _worker_connections;
		size_t _keepalive_timeout;
		std::vector<ServerConfig> _http;
		void setErrorLog(std::vector<confToken>		&tokens, size_t lineNum);
		void setAccessLog(std::vector<confToken>	&tokens, size_t lineNum);
		void setWorkConn(std::vector<confToken>		&tokens, size_t lineNum);
		void setTimeout(std::vector<confToken>		&tokens, size_t lineNum);
		void setHttp(std::vector<confToken>			&tokens, size_t lineNum);
		
		public:
		MainConfig() = delete;
		MainConfig(std::string &filename);
		~MainConfig() = default;
		MainConfig(MainConfig& src) = delete;
		MainConfig& operator=(MainConfig &src) = delete;
		void	checkValues(void);
		void	setDefaultValues(void);
		void	printConfig();
		
	};
	void tokenizeConfig(std::vector<confToken> &tokens, std::string &filename);
	void rmComment(std::string &line);
	void prepareLine(std::string &line, size_t lineNum);
	void collectContext(std::vector<confToken> &tokens, std::vector<confToken>::iterator it, std::vector<confToken> &context);
	void printConfTokens(std::vector<confToken>	&tokens);
	void typesortTokens(std::vector<confToken> &tokens);
	void OpenLogFile(std::string path, std::ofstream &file);
	void myLog(std::string type, std::string message);
	template <typename type> void parseTokens(std::vector<confToken> &tokens, std::map <std::string, void(type::*)(std::vector<confToken> &, size_t lineNum)> directives, type &obj);
	#include "../src/ConfigTokens.tpp"


#endif
