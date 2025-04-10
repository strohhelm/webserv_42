

#include <iostream>
#include <fstream>
#include <map>
#include <vector>


struct routeConfig
{
	std::vector<std::string>	_methods; //Define a list of accepted HTTP methods for the route
	int 						_redirectCode;
	std::string					_redirectPath; //Define a HTTP redirection
	std::string					_rootDir; //Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet).
	bool						_dirListing; //Turn on or off directory listing.
	std::string					_defaultFile; //Set a default file to answer if the request is a directory.
	std::string					_uploadPath; //Make the route able to accept uploaded files and configure where they should be saved.
	std::vector<std::string>	_cgiExtension; //Execute CGI based on certain file extension (for example .php)

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
		int							_port; //default set to 80
		std::vector<std::string>	_serverNames; //default set to 127.0.0.1?

		std::string 				_rootDir;
		std::string 				_indexFile; //default set to index.html?

		std::map<int, std::string>	_errorPage; // 404 ./var/www/html/40x.html
		std::map<std::string, routeConfig> _routes; // path and config

	public:
		ServerConfig();
		~ServerConfig();
		void setUrl(const std::vector<std::string>& serverNames ,const int& port);
		class ConfigurationFileException : public std::exception
		{
			public:
				const char* what() const noexcept override;
		};
		class ConfigurationException : public std::exception
		{
			public:
				const char* what() const noexcept override;
		};
		class ConfigurationException : public std::exception
		{
			public:
				const char* what() const noexcept override;
		};
};

void getConfiguration(std::vector<ServerConfig> &config);

/*
server {
    listen 80;
    server_name www.example.com example.com;

    location / {
        root /var/www/example;
        index index.html;
    }
}
*/