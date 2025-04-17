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


void Post::handleUpload()
{
	size_t begin = body.find("filename");
	begin = body.find('"', begin) + 1;
	size_t end = body.find('"', begin);

	std::string filename = body.substr(begin, end - begin);
	std::cout << "filename: " << filename << std::endl;

	begin = content.find("boundary=") + 9;
	std::string boundary = content.substr(begin);

	std::cout << "boundary: " << boundary << std::endl;

	boundary = "--" + boundary;
	begin = body.find(boundary) + boundary.size();
	begin = body.find("\r\n\r\n", begin) + 4;
	boundary.append("--");
	end = body.find("\r\n" + boundary , begin);
	std::string content = body.substr(begin, end - begin);

	std::cout << "content: " << content << std::endl;
	std::cout << "end\n";

	std::ofstream output("./file_upload/" + filename, std::ios::binary);
	if (output.is_open())
	{
		output.write(content.c_str(), content.size());
		output.close();
	}

}

Post::Post(std::string path, std::string body, std::string encoding,
	int fd) : path(path), body(body), content(encoding), fd(fd)
{
	if (content.size())
	if (path == "/signup")
		handleSignup();
	if (path == "/login")
		handleLogin();
	if (path == "/upload")
		handleUpload();
}
