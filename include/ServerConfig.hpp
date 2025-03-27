

#include <iostream>
#include <map>

class ServerConfig
{
    private:
        int         _port;      //default set to 80
        std::string _rootDir;   
        std::string _serverName;
        std::string _indexFile; //default set to index.html?
        int         _client_max_body_size;

        bool        _autoindex; //default set to false

        std::map<int, std::string> _errorPage; // 404 ./var/www/html/40x.html
        std::map<std::string, std::string> location;


    public:


};


// location
/*
    _httpMethods GET POST DELETE
    _httpRedirection

*/