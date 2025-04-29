#include "../include/Post.hpp"

void HttpRequest::postRespond()
{
	_state.reset();
	// if (_state._uploadFile.is_open())
	// 	_state._uploadFile.close();
	std::string html_content = readFileContent("www/upload/upload.html");
	sendResponse(200, html_content);
}

void HttpRequest::checkFilename(std::filesystem::path filePath)
{
	int i = 1;
	while (std::filesystem::exists(filePath))
	{
		size_t end = filePath.stem().string().find("_", 0);
		std::string new_stem;
		if (end == std::string::npos)
			new_stem = filePath.stem().string() + "_" + std::to_string(i);
		else
			new_stem = filePath.stem().string().substr(0, end) + "_" + std::to_string(i);
		filePath.replace_filename(new_stem + filePath.extension().string());
		i++;
	}
	_state._filename = filePath.filename().string();
}

int HttpRequest::dirSetup()
{
	_path = "client_" + std::to_string(_client_fd);
	if (_state._uploadMode)
	{
		std::cout << RED << "UPLOAD MODE SET" << RESET << std::endl;
		if (!std::filesystem::exists(_tempDir) && !std::filesystem::is_directory(_tempDir))
			std::filesystem::create_directory(_tempDir);
		if (!std::filesystem::exists(_tempDir / _path) && !std::filesystem::is_directory(_tempDir / _path))
			std::filesystem::create_directory(_tempDir / _path);
	}
	if (!std::filesystem::exists(_uploadDir) && !std::filesystem::is_directory(_uploadDir))
		std::filesystem::create_directory(_uploadDir);
	if (!std::filesystem::exists(_uploadDir / _path) && !std::filesystem::is_directory(_uploadDir / _path))
		std::filesystem::create_directory(_uploadDir / _path);
	return(1);
}	

int HttpRequest::extractInfo()
{
	size_t begin = 0;
	size_t end = 0;
	if(_state._openBoundary.empty())
	{
		begin = _contentHeader.find("boundary=", 0);
		if (begin == std::string::npos){
			std::cerr << "Couldnt find boundary in extract info" << std::endl; 
			return(0);
		}
		begin += 9;

		_state._openBoundary =  "--" + _contentHeader.substr(begin);
		_state._closeBoundary = _state._openBoundary + "--";
		std::cout << "OpenBoundary: " << _state._openBoundary << std::endl << "CloseBoundary: " << _state._closeBoundary << std::endl;
	}
	if (_state._filename.empty() && _rawBody.size())
	{
		begin = _rawBody.find("filename=\"", 0);
		if (begin == std::string::npos){
			std::cerr << "Couldnt find filename in extract info" << std::endl;
			return(0);

		}
		begin += 10;

		end = _rawBody.find('"', begin);
		if(end == std::string::npos){
			std::cerr << "Couldnt find filename end in extract info" << std::endl;
			return(0);
		}

		_state._filename = _rawBody.substr(begin, end - begin);
		std::cout << "filename: " << _state._filename << std::endl;
	}
	return (1);
}

int HttpRequest::extractContent()
{	
	size_t begin = 0;
	size_t end = 0;

	if (!_state._uploadMode)
	{
		if ((begin = _rawBody.find(_state._openBoundary + "\r\n\r\n", 0) + _state._openBoundary.size() + 4) == std::string::npos){
			std::cerr << "Couldnt find open boundary in _rawBody in extract content" << std::endl;
			return(0);
		}
		if ((end = _rawBody.find("\r\n" + _state._closeBoundary, begin)) == std::string::npos){
			std::cerr << "Couldnt find close boundary in _rawBody in extract content" << std::endl;
			return(0);
		}
		_fileContent = _rawBody.substr(begin, end - begin);
	}
	else if (_state._uploadMode && !_state._filename.empty())
	{
		if(!_state._uploadFile.is_open())
		{
			if ((begin = _rawBody.find(_state._openBoundary + "\r\n\r\n", 0) + _state._openBoundary.size() + 4) == std::string::npos){
				std::cerr << "Couldnt find open boundary in _rawBody in extract content" << std::endl;
				return(0);
			}
			_fileContent = _rawBody.substr(begin);
		}
		if(_state._uploadFile.is_open())
		{
			end = _rawBody.find("\r\n" + _state._closeBoundary, begin);
			if (end == std::string::npos)
				_fileContent = _rawBody;
			else
			{
				std::cout << GREEN << "Entire file recieved" << RESET << std::endl;
				_fileContent = _rawBody.substr(begin, end - begin);
				_done = true;
			}
			std::cout << _state._contentLength << " | " << _state._ContentBytesRecieved << std::endl;
		}
	}
	return(1);
}

int HttpRequest::writeContent()
{
	if (!_fileContent.empty())
	{	
		if (!_state._uploadMode)
		{
			checkFilename(_uploadDir / _path / _state._filename);
			std::ofstream output(_uploadDir / _path / _state._filename, std::ios::binary);
			if (output.is_open())
			{
				output.write(_fileContent.c_str(), _fileContent.size());
				output.close();
				postRespond();
			}
			else{
				std::cerr << "Unable to open file." << std::endl;
				return (0);
			}
		}
		else
		{
			if (!_state._uploadFile.is_open())
			{
				checkFilename(_tempDir / _path / _state._filename);
				_state._uploadFile.open(_tempDir / _path / _state._filename, std::ios::binary | std::ios::app);
			}
			if (_state._uploadFile.is_open())
			{
				_state._uploadFile << _fileContent;
			}
			else{
				std::cerr << "Unable to open file." << std::endl;
				return (0);
			}
			if (_done)
			{
				std::string curr_name = _state._filename;
				checkFilename(_uploadDir / _path / _state._filename);
				std::rename((_tempDir / _path / curr_name).c_str(), (_uploadDir / _path / _state._filename).c_str());
				postRespond();
			}
		}
	}
	return (1);
}

void HttpRequest::handleUpload()
{
	if(!_rawBody.empty())
		_state._ContentBytesRecieved += _rawBody.size();
	std::cout << RED << "Entered Post" << RESET << std::endl;
	if (!extractInfo()){
		std::cout << "Returned from extractInfo" << std::endl;
		return;
	}
	if (!dirSetup()){
		std::cout << "Returned from dirSetup" << std::endl;
		return;
	}
	if (!extractContent()){
		std::cout << "Returned from extractContent" << std::endl;
		return;
	}
	if (!writeContent()){
		std::cout << "Returned from writeContent" << std::endl;
		return;
	}
}

// Post::Post(std::string path, std::string _rawBody, std::string encoding, const int &fd, RequestState &_state) : 
// path(path), _rawBody(_rawBody), _contentHeader(encoding), fd(fd), _state(_state)
// {
// 	if (path == "/upload")
// 		handleUpload();
// }
