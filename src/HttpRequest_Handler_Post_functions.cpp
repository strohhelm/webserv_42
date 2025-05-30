#include "../include/HttpRequest.hpp"
#include "../include/ServerConfig.hpp"

void HttpRequest::postRespond()
{
	reset();
	sendRedirectResponse(204, "/" );
}

void HttpRequest::checkFilename(std::filesystem::path filePath)
{
	int i = 1;
	while (std::filesystem::exists(filePath))
	{
		size_t end = filePath.stem().string().rfind("_");
		std::string new_stem;
		if (end == std::string::npos || ((std::count(_fileName.begin(), _fileName.end(), '_')) == (std::count(_state._filename.begin(), _state._filename.end(), '_'))))
			new_stem = filePath.stem().string() + "_" + std::to_string(i);
		else
			new_stem = filePath.stem().string().substr(0, end) + "_" + std::to_string(i);
		filePath.replace_filename(new_stem + filePath.extension().string());
		i++;
		_fileName = filePath.filename().string();
	}
}

int HttpRequest::dirSetup()
{
	_path = "client_" + std::to_string(_client_fd);
	try
	{
		if (_state._uploadMode)
		{
			if (!std::filesystem::exists(_tempDir) && !std::filesystem::is_directory(_tempDir))
				std::filesystem::create_directory(_tempDir);
			if (!std::filesystem::exists(_tempDir / _path) && !std::filesystem::is_directory(_tempDir / _path))
				std::filesystem::create_directory(_tempDir / _path);
		}
		if (!std::filesystem::exists(_uploadDir) && !std::filesystem::is_directory(_uploadDir))
			std::filesystem::create_directory(_uploadDir);
		if (!std::filesystem::exists(_uploadDir / _path) && !std::filesystem::is_directory(_uploadDir / _path))
			std::filesystem::create_directory(_uploadDir / _path);
	}
	catch(const std::exception& e)
	{
		std::cout <<BG_BRIGHT_RED<<"SETUP UPLOAD DIRECTORY: "<< e.what() <<RESET<< '\n';
		return (0);
	}
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
			if(debug){std::cout << "Couldnt find boundary in extract info" << std::endl;}
			return(0);
		}
		begin += 9;

		_state._openBoundary =  "--" + _contentHeader.substr(begin);
		_state._closeBoundary = _state._openBoundary + "--";
		if(debug){std::cout << "OpenBoundary: " << _state._openBoundary << std::endl << "CloseBoundary: " << _state._closeBoundary << std::endl;}
	}
	if (_state._filename.empty() && _state._buffer.size())
	{
		begin = _state._buffer.find("filename=\"", 0);
		if (begin == std::string::npos){
			if(debug){std::cout << "Couldnt find filename in extract info" << std::endl;}
			return(0);

		}
		begin += 10;

		end = _state._buffer.find('"', begin);
		if(end == std::string::npos){
			if(debug){std::cout << "Couldnt find filename end in extract info" << std::endl;}
			return(0);
		}

		_state._filename = _state._buffer.substr(begin, end - begin);
		_fileName = _state._filename;
		if(debug){std::cout << "filename: " << _state._filename << std::endl;}
		if (_state._filename.empty())
		{
			postRespond();
			return(1);
		}
	}
	return (1);
}

int HttpRequest::extractContent()
{	
	size_t begin = 0;
	size_t end = 0;
	if (_state._buffer.empty())
		return(1);
	if (!_state._uploadMode)
	{
		if ((begin = _state._buffer.find(_state._openBoundary, 0)) == std::string::npos){
			if(debug){std::cout << "Couldnt find open boundary in _state._buffer in extract content" << std::endl;}
			return(0);
		}
		begin += _state._openBoundary.size();
		if ((begin = _state._buffer.find("\r\n\r\n", begin)) == std::string::npos){
			if(debug){std::cout << "Couldnt find open boundary in _state._buffer in extract content" << std::endl;}
			return(0);
		}
		begin += 4;
		if ((end = _state._buffer.find("\r\n" + _state._closeBoundary, begin)) == std::string::npos){
			if(debug){std::cout << "Couldnt find close boundary in _state._buffer in extract content" << std::endl;}
			return(0);
		}
		_fileContent = _state._buffer.substr(begin, end - begin);
		_state._uploadComplete = true;
	}
	else if (_state._uploadMode && !_state._filename.empty())
	{
		if(!_state._uploadFile.is_open())
		{
			if ((begin = _state._buffer.find(_state._openBoundary, 0)) == std::string::npos){
				if(debug){std::cout << "Couldnt find open boundary in _state._buffer in extract content" << std::endl;}
				return(0);
			}
			begin += _state._openBoundary.size();
			if ((begin = _state._buffer.find("\r\n\r\n", begin)) == std::string::npos){
				if(debug){std::cout << "Couldnt find open boundary in _state._buffer in extract content" << std::endl;}
				return(0);
			}
			begin += 4;
			_fileContent = _state._buffer.substr(begin);
		}
		if(_state._uploadFile.is_open())
		{
			end = _state._buffer.find("\r\n" + _state._closeBoundary, begin);
			if (end == std::string::npos)
				_fileContent = _state._buffer;
			else
			{
				if(debug){std::cout << GREEN << "Entire file recieved" << RESET << std::endl;}
				_fileContent = _state._buffer.substr(begin, end - begin);
				_state._uploadComplete = true;
			}

			if(debug){std::cout << _state._contentLength << " | " << _state._ContentBytesRecieved << std::endl;}
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
			checkFilename(_uploadDir / _path / _fileName);
			std::ofstream output(_uploadDir / _path / _fileName, std::ios::binary);
			if (output.is_open())
			{
				output << _fileContent;
				if (!output)
				{
					if(debug){std::cout << "File stream error." << std::endl;}
					output.close();
					return(0);
				}
				output.close();
				postRespond();
			}
			else
			{
				if(debug){std::cout << "Unable to open file." << std::endl;}
				return (0);
			}
		}
		else
		{
			if (!_state._uploadFile.is_open())
			{
				checkFilename(_tempDir / _path / _fileName);
				_state._uploadFile.open(_tempDir / _path / _fileName, std::ios::binary | std::ios::app);
			}
			if (_state._uploadFile.is_open())
			{
				_state._uploadFile << _fileContent;
				if (!_state._uploadFile)
				{
					if(debug){std::cout << "File write error." << std::endl;}
					return(0);
				}
				_state._uploadFile.flush();
				if (!_state._uploadFile)
				{
					if(debug){std::cout << "File flush error." << std::endl;}
					return(0);
				}
			}
			else
			{
				if(debug){std::cout << "Unable to open temp file." << std::endl;}
				return (0);
			}
			if (_state._uploadComplete)
			{
				std::string curr_name = _fileName;
				checkFilename(_uploadDir / _path / _fileName);
				if (curr_name != _fileName)
					std::rename((_tempDir / _path / curr_name).c_str(), (_tempDir / _path / _fileName).c_str());
				std::rename((_tempDir / _path / _fileName).c_str(), (_uploadDir / _path / _fileName).c_str());
				postRespond();
			}
		}
	}
	_fileContent.clear();
	return (1);
}

void HttpRequest::handleUpload()
{
	if (_state._isCgiPost)
		return;
	if(!_state._buffer.empty())
		_state._ContentBytesRecieved += _state._buffer.size();
	if(debug){std::cout << GREEN << "Entered hadleUpload" << RESET << std::endl;}
	try
	{
		if (!extractInfo()){
			if(debug){std::cout << "Returned from extractInfo" << std::endl;}
			sendErrorResponse(500);
			return;
		}
		if (!extractContent()){
			if(debug){std::cout << "Returned from extractContent" << std::endl;}
			sendErrorResponse(500);
			return;
		}
		if (!writeContent()){
			if(debug){std::cout << "Returned from writeContent" << std::endl;}
			sendErrorResponse(500);
			return;
		}
		if ((_state._uploadMode && !_state._uploadComplete) && _state._ContentBytesRecieved == _state._contentLength)
		{
			if(debug){std::cout << "Closing boundary extraction error." << std::endl;}
			sendErrorResponse(400);
			return ;
		}
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		sendErrorResponse(500);
		return ;
	}
	_state._buffer.clear();
}
