#include"../include/HttpRequest.hpp"



#include<unistd.h>
#include<stdio.h>


void HttpRequest::runCgiScriptGet(const int& client_fd, const std::string& fullPath)
{
	std::string scriptPath = fullPath;
	std::string queryString;

	size_t pos = fullPath.find('?');
	if (pos != std::string::npos) {
		scriptPath = fullPath.substr(0, pos); // "/index2.php"
		queryString = fullPath.substr(pos + 1); // "name=Alice&lang=de"
	}


	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		sendErrorResponse(client_fd, 500, "Pipe creation failed"); //???
		return;
	}
	
	pid_t pid = fork();
	if (pid < 0)
	{
		sendErrorResponse(client_fd, 500, "Fork failed"); //???
		return;
	}

	std::cout << BG_BRIGHT_MAGENTA << fullPath.c_str() << RESET << std::endl;
	std::cout << BG_BRIGHT_MAGENTA << scriptPath.c_str() << RESET << std::endl;
	std::cout << BG_BRIGHT_MAGENTA << queryString.c_str() << RESET << std::endl;
	if (pid == 0)
	{
		// CHILD: send stdout to pipe
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);

		const char* phpCgiPath = "/usr/bin/php-cgi";
		char* argv[] =
		{
			(char*)phpCgiPath,
			(char*)scriptPath.c_str(),
			NULL
		};
		
	
		std::vector<std::string> envStrings = {
			"REQUEST_METHOD=GET",
			"SCRIPT_FILENAME=" + scriptPath,
			"SCRIPT_NAME=" + scriptPath,
			"QUERY_STRING=" + queryString,
			"GATEWAY_INTERFACE=CGI/1.1",
			"SERVER_PROTOCOL=HTTP/1.1",
			"REDIRECT_STATUS=200"
		};
		
		std::vector<char*> envp;
		for (auto& s : envStrings)
			envp.push_back(&s[0]);
		envp.push_back(nullptr);
		

		execve(phpCgiPath, argv, envp.data());
		exit(1); // only reached if exec fails
	}
	else
	{
		// PARENT: read from pipe
		close(pipefd[1]); // close write end

		std::string cgiOutput;
		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		{
			cgiOutput.append(buffer, bytesRead);
		}
		close(pipefd[0]);
	
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
		httpResponse += "Content-Type: text/html\r\n";
		httpResponse += "Content-Length: " + std::to_string(cgiOutput.size()) + "\r\n";
		httpResponse += "\r\n";
		httpResponse += cgiOutput;
	
		send(client_fd, httpResponse.c_str(), httpResponse.size(), 0);
	}
}
