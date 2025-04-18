
#include "../include/Colors.hpp"
#include <arpa/inet.h> // send()
#include "../include/ServerConfig.hpp"


void CGI::setCgiParameter(const int& client_fd, ServerConfig& config, std::string& requestPath, std::string& cgiPath)
{
	_client_fd = client_fd;
	_config = &config;
	_requestPath = requestPath;
	_phpCgiPathStr = cgiPath;
}

void CGI::tokenizePath(void)
{

	size_t pos = _requestPath.find('?');
	if(pos != std::string::npos)
	{
		_scriptPath = _requestPath.substr(0, pos); // "/index2.php"
		_queryString = _requestPath.substr(pos + 1); // "name=Alice&lang=de"
	}
	else
	{
		// ?????
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

	
	// std::cout << BG_RED << _phpCgiPath << RESET << std::endl;

	_phpCgiPath = _phpCgiPathStr.c_str();
	

	_argv[0] = (char*)_phpCgiPath;
	_argv[1] = (char*)_fullPath.c_str();
	_argv[2] = nullptr;

	// std::cout << "_argv\n"
	// 			<< "_argv[0] = " << _argv[0]
	// 			<< "_argv[1] = " << _argv[1]
	// 			<< "_argv[2] = " << _argv[2]
	// 			<< std::endl;

}

int	CGI::createPipes()
{
	if(pipe(_parent.data()) == -1)
	{
		return 1;
	}
	if(pipe(_child.data()) == -1)
	{
		closePipesFromFd(_parent);
		return 1;
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
}

void CGI::buildEnvStrings(std::string method, std::string rawBody)
{
	_envStrings = {
		"GATEWAY_INTERFACE=CGI/1.1",
		"REDIRECT_STATUS=200",
		"REQUEST_METHOD=" + method,
		"SCRIPT_FILENAME=" + _config->_rootDir + _scriptPath, //www/get.php
		"SCRIPT_NAME=" + _scriptPath, ///get.php
		"SERVER_PROTOCOL=HTTP/1.1",
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

	execve(_phpCgiPath, _argv, _envp.data());

	std::cerr << "execve failed: " << std::endl;
	exit(1);


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
	if (method == "POST" && !rawBody.size())
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

	send(_client_fd, httpResponse.c_str(), httpResponse.size(), 0);
}

// std::array<int, 3> a = {1, 2, 3};
void CGI::execute(std::string method, std::string rawBody)
{
	// init fds
	_parent = {-1, -1};
	_child = {-1, -1};

	if(createPipes())
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