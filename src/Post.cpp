#include "../include/Post.hpp"

std::map<std::string, std::string> _db;

void Post::handleSignup()
{
    std::cout << "- New User -" << std::endl;
    size_t pos = 0;
    size_t begin = body.find('=', pos) + 1;
    size_t end = body.find('&', begin);

    std::string usr = body.substr(begin , end - begin);
    std::cout << usr << std::endl;
    
    begin = body.find('=', end) + 1;
    std::string pw = body.substr(begin , body.size() - begin);
    std::cout << pw << std::endl;

    if (!usr.size() || !pw.size())
    {
		sendErrorResponse(fd, 420, "Please fill out all form fields");
    }

    if(_db.find(usr) == _db.end())
    {
        std::cout << "Username available!" << std::endl;
        _db.insert({usr, pw});

        std::string content = readFileContent("www/login.html");
	    sendResponse(fd, 200, content);	
    }
    else
    {
		sendErrorResponse(fd, 420, "Username already taken");
    }

}

Post::Post(std::string path, std::string body, std::string encoding, int fd) : path(path), body(body), fd(fd)
{
    if (encoding.size())
	if (path == "/signup")
        handleSignup();
    if (path == "/login")
		sendErrorResponse(fd, 420, "Under construction");
}
