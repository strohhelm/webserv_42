
#include "../include/Colors.hpp"
#include <arpa/inet.h> // send()
#include "../include/ServerConfig.hpp"
#include <cerrno> // needs to be removed when done only for debugging
#include <cstring> // needs to be removed when done only for debugging


void CGI::setCgiParameter(const int& client_fd, ServerConfig& config, std::string& requestPath, std::string& cgiPath)
{
	_client_fd = client_fd;
	_config = &config;
	_requestPath = requestPath;
	_phpCgiPathStr = cgiPath;
	std::cout << RED << _phpCgiPathStr << RESET << std::endl;
	std::cout << RED << _requestPath << RESET << std::endl;
}

void CGI::tokenizePath(void)
{
	
	size_t pos = _requestPath.find('?');
	if(pos != std::string::npos) // only at get
	{
		_scriptPath = _requestPath.substr(0, pos); // "/index2.php"
		_queryString = _requestPath.substr(pos + 1); // "name=Alice&lang=de"
	}
	else // only at Post
	{
		_scriptPath = _requestPath;
	}
	// std::cout << "CGI _requestPath: " << _requestPath << std::endl;
	// std::cout << "CGI _fullPath: " << _fullPath << std::endl;
	// std::cout << "SCRIPT_FILENAME: " << _fullPath << std::endl;
	// std::cout << "Checking file: " << (access(_fullPath.c_str(), F_OK) == 0 ? "Exists" : "DOES NOT EXIST") << std::endl;
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
	// check before closing default -1 to avoid error
	close(_parent[READ_FD]);
	close(_parent[WRITE_FD]);
	close(_child[READ_FD]);
	close(_child[WRITE_FD]);	
}

void	CGI::setArgv(void)
{
	// _phpCgiPathStr = this->_config->getCgiPath();
	// _phpCgiPath = _phpCgiPathStr.c_str();
	_fullPath = _config->_rootDir + _scriptPath;
	
	_argv[0] = (char*)_phpCgiPathStr.c_str();
	_argv[1] = (char*)_fullPath.c_str();
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
	
	// debugging
	for (char* env : _envp)
	{
		if (env) std::cerr << "env: " << env << std::endl;
	}

}

void CGI::buildEnvStrings(std::string method, std::string rawBody)
{
	_envStrings = {
		"GATEWAY_INTERFACE=CGI/1.1",
		"REDIRECT_STATUS=200",
		"REQUEST_METHOD=" + method,
		"SCRIPT_FILENAME=" + _config->_rootDir + _scriptPath, //www/get.php
		"SCRIPT_NAME=" + _scriptPath, ///get.php
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


void CGI::handleChildProcess(std::string method, std::string rawBody)
{
	setPipeToRead(_parent[READ_FD]); // return value of dup2 check
	setPipeToWrite(_child[WRITE_FD]);
	closeAllPipes();

	setArgv();

	buildEnvStrings(method, rawBody);

	convertEnvStringsToChar();


	std::cerr << "Running: " << _phpCgiPathStr << " with script " << _argv[1] << std::endl;


	execve(_phpCgiPathStr.c_str(), _argv, _envp.data());

	// std::cerr << "execve failed: " << std::endl;
	std::cerr << "argv[0]: " << _argv[0] << "\nargv[1]: " << _argv[1] << std::endl;

	std::cerr << "execve failed: " << strerror(errno) << std::endl;

	exit(1); //?!?!?!?!?!?!?!
}

std::string CGI::readCgiOutput(void)
{
	std::string cgiOutput;
	char buffer[1024];
	ssize_t bytesRead;

	while ((bytesRead = read(_child[READ_FD], buffer, sizeof(buffer))) > 0) //check returnvalue?!?!?
	{
		cgiOutput.append(buffer, bytesRead);
	}
	return cgiOutput;
}

void	CGI::sendPostDataToChild(std::string method, std::string rawBody)
{
	// Send POST data if any -> neeeds to be tested!?!?!?!?!
	if (method == "POST" && !rawBody.empty())
	{
		write(_parent[WRITE_FD], rawBody.c_str(), rawBody.size()); //check returnvalue?!?!?
	}
	// done writing
}



void CGI::handleParentProcess(std::string method, std::string rawBody)
{
	close(_parent[READ_FD]); // Parent doesn't need to read from this
	close(_child[WRITE_FD]); // Parent doesn't need to write to this
	
	sendPostDataToChild(method, rawBody);
	close(_parent[WRITE_FD]);

	std::string cgiOutput = readCgiOutput();
	close(_child[READ_FD]);

	std::string httpResponse = "HTTP/1.1 200 OK\r\n";
	httpResponse += "Content-Length: " + std::to_string(cgiOutput.size()) + "\r\n";
	httpResponse += "Content-Type: text/html\r\n";
	httpResponse += "\r\n";
	httpResponse += cgiOutput;

	/**********************************************************/
	std::cout << "SCRIPT_FILENAME=" << _config->_rootDir + _scriptPath << std::endl;	//www/get.php
	std::cout << "SCRIPT_NAME=" << _scriptPath << std::endl; ///get.php
	std::cout << "argv\n"
				<< _phpCgiPathStr.c_str() << "\n"
				<< _config->_rootDir + _scriptPath << "\n"
				<< "---\n"
				<< std::endl;

	/**********************************************************/
	std::cout << "CGI raw output:\n" << cgiOutput << std::endl;


	send(_client_fd, httpResponse.c_str(), httpResponse.size(), 0);
}

// std::array<int, 3> a = {1, 2, 3};
void CGI::execute(std::string method, std::string rawBody)
{
	std::cout << "CGI" << std::endl;
	// init fds
	_parent = {-1, -1};
	_child = {-1, -1};

	if(createPipes() < 0)
	{
		// sendErrorResponse(client_fd, 500, "Pipe creation failed"); //???
		return;
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		closeAllPipes();
		// sendErrorResponse(client_fd, 500, "Fork failed");
		return;
	}
	// 



	// 
	if(pid == 0)
	{
		handleChildProcess(method, rawBody);
	}
	else
	{
		handleParentProcess(method, rawBody);
	}

	closeAllPipes();
}