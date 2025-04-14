#include "../include/Post.hpp"

std::map<std::string, std::string> _db;

userdata Post::urlDecode()
{
	size_t	pos = 0;
	size_t	begin;
	size_t	end;
	userdata re;

	begin = body.find('=', pos) + 1;
	end = body.find('&', begin);
	re.username = body.substr(begin, end - begin);
	std::cout << re.username << std::endl;
	begin = body.find('=', end) + 1;
	re.password = body.substr(begin, body.size() - begin);
	std::cout << re.password << std::endl;

	return (re);
}

void Post::handleSignup()
{
	std::cout << "- New User -" << std::endl;

	userdata signup = urlDecode();

	if (!signup.username.size() || !signup.password.size())
		sendErrorResponse(fd, 420, "Please fill out all form fields");
	if (_db.find(signup.username) == _db.end())
	{
		std::cout << "Username available!" << std::endl;
		_db.insert({signup.username, signup.password});
		std::string content = readFileContent("www/login.html");
		sendResponse(fd, 200, content);
	}
	else
		sendErrorResponse(fd, 420, "Username already taken");
}

void Post::handleLogin()
{
	std::cout << "- Login Request -" << std::endl;

	userdata login = urlDecode();

	if (!login.username.size() || !login.password.size())
		sendErrorResponse(fd, 420, "Please fill out all form fields");
	auto it = _db.find(login.username);
	if (it == _db.end())
		sendErrorResponse(fd, 420, "No user with this name");
	else
	{
		if (it->second == login.password)
			sendErrorResponse(fd, 420, "Login successfull");
	}
}

Post::Post(std::string path, std::string body, std::string encoding,
	int fd) : path(path), body(body), fd(fd)
{
	if (encoding.size())
	if (path == "/signup")
		handleSignup();
	if (path == "/login")
		handleLogin();
}
