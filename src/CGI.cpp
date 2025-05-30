
#include <arpa/inet.h> // send()
// #include "../include/ServerConfig.hpp"
#include "../include/SimpleServer.hpp"
#include <cerrno> // needs to be removed when done only for debugging
#include <cstring> // needs to be removed when done only for debugging
#include <fcntl.h>

std::string HttpRequest::extractQueryString(std::string& request)
{
	size_t pos = request.find('?');
	if(pos != std::string::npos) // only at get
	{
		return request.substr(pos + 1);
	}
	return "";
}

void CGI::setCgiParameter(const int& client_fd, ServerConfig& config, std::string& requestPath, std::string& cgiPath, std::string& query)
{
	_client_fd = client_fd;
	_config = &config;
	_requestPath = requestPath;
	_scriptPath = requestPath;
	_phpCgiPathStr = cgiPath;
	_queryString = query;
	if (debug)std::cout << ORANGE << _phpCgiPathStr << RESET << std::endl;
	if (debug)std::cout << ORANGE << _requestPath << RESET << std::endl;
}

void CGI::tokenizePath(void)
{
	size_t pos = _requestPath.find('?');
	std::cout << "CGI _requestPath: " << _requestPath << std::endl;
	if(pos != std::string::npos) // only at get
	{
		_scriptPath = _requestPath.substr(0, pos); // "/index2.php"
		_queryString = _requestPath.substr(pos + 1); // "name=Alice&lang=de"
		// std::cout << "CGI _scriptPath: " << _scriptPath << std::endl;
		// std::cout << "CGI _queryString: " << _queryString << std::endl;
	}
	else // only at Post
	{
		_scriptPath = _requestPath;
		// std::cout << "CGI _scriptPath: " << _scriptPath << std::endl;
	}
	if (debug)std::cout <<YELLOW<< "CGI _fullPath: " << _fullPath <<RESET<< std::endl;
	if (debug)std::cout <<YELLOW<< "SCRIPT_path: " << _scriptPath <<RESET<< std::endl;
	if (debug)std::cout <<YELLOW<< "Checking file: " << (access(_fullPath.c_str(), F_OK) == 0 ? "Exists" : "DOES NOT EXIST") <<RESET<< std::endl;
}


/*
fd[2][2];
fd[x][0] = read
fd[x][1] = write
*/

/*
#define STDIN_FILENO  0  // standard input
#define STDOUT_FILENO 1  // standard output
#define STDERR_FILENO 2  // standard error
*/


void CGI::closePipesFromFd(std::array<int, 2>& fd)
{
	close(fd[READ_FD]);
	close(fd[WRITE_FD]);
}

void	CGI::closeAllPipes(void)
{
	if(_parent[READ_FD] != -1)
		close(_parent[READ_FD]);
	if(_parent[WRITE_FD] != -1)
		close(_parent[WRITE_FD]);
	if(_child[READ_FD] != -1)
		close(_child[READ_FD]);
	if(_child[WRITE_FD] != -1)
		close(_child[WRITE_FD]);
}

void	CGI::setArgv(void)
{
	// _phpCgiPathStr = this->_config->getCgiPath();
	// _phpCgiPath = _phpCgiPathStr.c_str();
	// _fullPath = _config->_rootDir + _scriptPath;
	
	_argv[0] = (char*)_phpCgiPathStr.c_str();
	_argv[1] = (char*)_scriptPath.c_str();
	_argv[2] = nullptr;
}

int	CGI::createPipes()
{
	if(pipe(_parent.data()) == -1)
	{
		return -1;
	}
	if(pipe(_child.data()) == -1)
	{
		closePipesFromFd(_parent);
		return -1;
	}
	return 0;
}

void	CGI::setPipeToRead(int fd)
{
	dup2(fd, STDIN_FILENO);
}

void	CGI::setPipeToWrite(int fd)
{
	dup2(fd, STDOUT_FILENO);	
}

void CGI::convertEnvStringsToChar(void)
{
	for (auto& s : _envStrings)
	{
		_envp.push_back(&s[0]);
	}
	_envp.push_back(nullptr);
	
	// // debugging
	// for (char* env : _envp)
	// {
	// 	if (env) std::cerr << "env: " << env << std::endl;
	// }
}

void CGI::buildEnvStrings(std::string method, std::string rawBody)
{
	_envStrings = {
		"GATEWAY_INTERFACE=CGI/1.1",
		"REDIRECT_STATUS=200",
		"SERVER_PROTOCOL=HTTP1.1",
		"REQUEST_METHOD=" + method,
		"SCRIPT_FILENAME=" + _scriptPath,	//www/get.php
		"SCRIPT_NAME=" + _scriptPath, 		// get.php
	};
	
	if (method == "GET") {
		_envStrings.push_back("QUERY_STRING=" + _queryString); // everything after the ?	
	}
	else if (method == "POST")
	{
		_envStrings.push_back("CONTENT_LENGTH=" + std::to_string(rawBody.size()));
		_envStrings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
	}
}


int CGI::handleChildProcess(std::string method, std::string rawBody)
{
	setPipeToRead(_parent[READ_FD]); // return value of dup2 check
	setPipeToWrite(_child[WRITE_FD]);
	closeAllPipes();

	setArgv();
	buildEnvStrings(method, rawBody);
	convertEnvStringsToChar();

	std::cerr << "Running: " << _phpCgiPathStr << " with script " << _argv[1] << std::endl;

	execve(_phpCgiPathStr.c_str(), _argv, _envp.data());

	std::cerr << "execve failed: " << std::endl;
	return 1;
}

std::string CGI::readCgiOutput(int& error)
{
	std::string cgiOutput;
	char buffer[1024];
	ssize_t bytes;
	
	fcntl(_child[READ_FD], F_SETFL, O_NONBLOCK);
	struct pollfd reader = {_child[READ_FD], POLLIN, 0};

	int ret = poll(&reader, 1, 2000);
	if(ret == -1)
	{
		error = -1;
		return "";
	}
	if(ret == 0)
	{
		error = 0;
		return "";
	}
	std::cout<<"poll: "<<ret<<std::endl;

	while (1)
	{
		bytes = read(_child[READ_FD], buffer, sizeof(buffer));
		
		if(debug){std::cout<<"Bytes read: "<<bytes <<" "<< errno<<std::endl;}
		if(bytes == -1)
		{
			error = -1;
			return "";
		}
		if(bytes == 0)
		{
			return cgiOutput;
		}
		cgiOutput.append(buffer, bytes);
		if (bytes < (ssize_t)sizeof(buffer))
			return (cgiOutput);
	}
	return cgiOutput;
}

void	CGI::sendPostDataToChild(std::string method, std::string rawBody, int& error)
{
	ssize_t bytes;
	if (method == "POST" && !rawBody.empty())
	{
		bytes = write(_parent[WRITE_FD], rawBody.c_str(), rawBody.size());
		if(bytes == -1)
		{
			error = -1;
			return ;
		}
		if(bytes == 0)
		{
			error = 0;
			return ;
		}
	}
}



int CGI::handleParentProcess(std::string method, std::string rawBody)
{
	int error = 1;
	close(_parent[READ_FD]); // Parent doesn't need to read from this
	close(_child[WRITE_FD]); // Parent doesn't need to write to this
	
	sendPostDataToChild(method, rawBody, error);
	if(error == -1 || error == 0)
		return 1;
	close(_parent[WRITE_FD]);
		
	std::string cgiOutput = readCgiOutput(error);
	if(error == -1 || error == 0)
		return 1;
	close(_child[READ_FD]);

	std::string httpResponse = "HTTP/1.1 200 OK\r\n";
	httpResponse += "Content-Length: " + std::to_string(cgiOutput.size()) + "\r\n";
	httpResponse += "Content-Type: text/html\r\n";
	httpResponse += "\r\n";
	httpResponse += cgiOutput;

	/**********************************************************/
	if (debug)std::cout << "SCRIPT_FILENAME=" << _config->_rootDir + _scriptPath << std::endl;	//www/get.php
	if (debug)std::cout << "SCRIPT_NAME=" << _scriptPath << std::endl; ///get.php
	if (debug)std::cout << "argv\n"
				<< _phpCgiPathStr.c_str() << "\n"
				<< _config->_rootDir + _scriptPath << "\n"
				<< "---\n"
				<< std::endl;

	/**********************************************************/
	if (debug)std::cout << "CGI raw output:\n" << cgiOutput << std::endl;

	int bytestosend = httpResponse.length();
	int bytessent = send(_client_fd, httpResponse.c_str(), httpResponse.size(), 0); //todo 
	if (bytessent != bytestosend)
		std::cout<<BG_BRIGHT_RED<<"SEND FAILED in CGI parentprocess"<<RESET<<std::endl;
	if(bytessent == -1 || bytessent == 0)
		return 1;
	return 0;
}

// std::array<int, 3> a = {1, 2, 3};
int CGI::execute(std::string method, std::string rawBody)
{
	if (debug)std::cout << "CGI" << std::endl;
	// init fds
	_parent = {-1, -1};
	_child = {-1, -1};

	if(createPipes() < 0)
	{
		return 500;
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		closeAllPipes();
		return 500;
	}
	if(pid == 0)
	{
		if(handleChildProcess(method, rawBody))
		{
			closeAllPipes();
			return 500;
		}
	}
	else
	{
		if(handleParentProcess(method, rawBody))
		{
			closeAllPipes();
			return 500;
		}
	}
	closeAllPipes();
	return 0;
}