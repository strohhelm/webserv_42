#include "../include/Post.hpp"

// std::map<std::string, std::string> _db;

// userdata Post::urlDecode()
// {
// 	size_t	pos = 0;
// 	size_t	begin;
// 	size_t	end;
// 	userdata re;

// 	begin = body.find('=', pos) + 1;
// 	end = body.find('&', begin);
// 	re.username = body.substr(begin, end - begin);
// 	std::cout << re.username << std::endl;
// 	begin = body.find('=', end) + 1;
// 	re.password = body.substr(begin, body.size() - begin);
// 	std::cout << re.password << std::endl;

// 	return (re);
// }

// void Post::handleSignup()
// {
// 	std::cout << "- New User -" << std::endl;

// 	userdata signup = urlDecode();

// 	if (!signup.username.size() || !signup.password.size())
// 		sendErrorResponse(fd, 420);
// 	if (_db.find(signup.username) == _db.end())
// 	{
// 		std::cout << "Username available!" << std::endl;
// 		_db.insert({signup.username, signup.password});
// 		std::string content = readFileContent("www/login/login.html");
// 		sendResponse(fd, 200, content);
// 	}
// 	else
// 		sendErrorResponse(fd, 420);
// }

// void Post::handleLogin()
// {
// 	std::cout << "- Login Request -" << std::endl;

// 	userdata login = urlDecode();

// 	if (!login.username.size() || !login.password.size())
// 		sendErrorResponse(fd, 420);
// 	auto it = _db.find(login.username);
// 	if (it == _db.end())
// 		sendErrorResponse(fd, 420);
// 	else
// 	{
// 		if (it->second == login.password)
// 			sendErrorResponse(fd, 420);
// 	}
// }


void Post::dirSetup()
{
	_fdPath = "/client_" + std::to_string(fd);
	if (_state._uploadMode)
	{
		std::cout << RED << "UPLOAD MODE SET" << RESET << std::endl;
		if (!std::filesystem::exists(_tempDir) && !std::filesystem::is_directory(_tempDir))
			std::filesystem::create_directory(_tempDir);
		if (!std::filesystem::exists(_tempDir + _fdPath) && !std::filesystem::is_directory(_tempDir + _fdPath))
			std::filesystem::create_directory(_tempDir + _fdPath);
	}
	if (!std::filesystem::exists(_uploadDir) && !std::filesystem::is_directory(_uploadDir))
		std::filesystem::create_directory(_uploadDir);
	if (!std::filesystem::exists(_uploadDir + _fdPath) && !std::filesystem::is_directory(_uploadDir + _fdPath))
		std::filesystem::create_directory(_uploadDir + _fdPath);

}	

void Post::extractInfo()
{
	if (_state._filename.empty() && !body.empty())
	{
		size_t begin = body.find("filename");
		begin = body.find('"', begin) + 1;
		size_t end = body.find('"', begin);
	
		_state._filename = body.substr(begin, end - begin);
		std::cout << "filename: " << _state._filename << std::endl;
	}
	if(_state._openBoundary.empty())
	{
		size_t begin = _contentHeader.find("boundary=") + 9;
		std::string boundary = _contentHeader.substr(begin);
	
		
		_state._openBoundary = "--" + boundary;
		// begin = body.find(boundary) + boundary.size();
		// begin = body.find("\r\n\r\n", begin) + 4;
		_state._closeBoundary = _state._openBoundary + "--";
		
	}
}


void Post::extractContent()
{	
	if (!_state._uploadMode)
	{
		size_t begin = body.find(_state._openBoundary) + _state._openBoundary.size();
		begin = body.find("\r\n\r\n", begin) + 4;
		size_t end = body.find("\r\n" + _state._closeBoundary, begin);
		_fileContent = body.substr(begin, end - begin);
	}
	else if (_state._uploadMode && !_state._filename.empty())
	{
		size_t begin = 0;

		if(!_state._uploadFile.is_open())
		{
			begin = body.find(_state._openBoundary) + _state._openBoundary.size();
			begin = body.find("\r\n\r\n", begin) + 4;
		}
		size_t end = body.find("\r\n" + _state._closeBoundary, begin);
		if (end == std::string::npos)
			_fileContent = body.substr(begin);
		else
		{
			std::cout << RED << "Entire file recieved" << RESET << std::endl;
			_fileContent = body.substr(begin, end - begin);
			_done = true;
			// std::cout << "boundary: " << _state._openBoundary << std::endl;
			// std::cout << "boundary: " << _state._closeBoundary << std::endl;
		}
		std::cout << _state._contentLength << " | " << _state._ContentBytesRecieved << std::endl;
		if (_state._contentLength == _state._ContentBytesRecieved)
		{
			std::cout << RED << "Entire file recieved" << RESET << std::endl;
			_done = true;
		}
	}
}

void Post::writeContent()
{
	if (!_fileContent.empty())
	{	
		if (!_state._uploadMode)
		{
			std::ofstream output(_uploadDir + _fdPath + '/' + _state._filename, std::ios::binary);
			if (output.is_open())
			{
				output.write(_fileContent.c_str(), _fileContent.size());
				output.close();
			}
				std::string html_content = readFileContent("www/upload/upload.html");
				sendResponse(fd, 200, html_content);
				_state.reset();
		}
		else
		{
			if (!_state._uploadFile.is_open())
				_state._uploadFile.open(_tempDir + _fdPath + '/' + _state._filename, std::ios::binary | std::ios::app);
			if (_state._uploadFile.is_open())
			{
				_state._uploadFile << _fileContent;
			}
			if (_done)
			{
				_state._uploadFile.close();
				std::string html_content = readFileContent("www/upload/upload.html");
				sendResponse(fd, 200, html_content);
				_state.reset();
			}
		}
	}
}

void Post::handleUpload()
{
	if(!body.empty())
		_state._ContentBytesRecieved += body.size();
	std::cout << RED << "Entered Post" << RESET << std::endl;
	extractInfo();
	dirSetup();
	extractContent();
	writeContent();

	// std::string html_content = readFileContent("www/upload/upload.html");
	// sendResponse(fd, 200, html_content);

}

Post::Post(std::string path, std::string body, std::string encoding, const int &fd, RequestState &_state) : 
path(path), body(body), _contentHeader(encoding), fd(fd), _state(_state)
{
	// if (path == "/signup")
	// 	handleSignup();
	// if (path == "/login")
	// 	handleLogin();
	if (path == "/upload")
		handleUpload();
}
