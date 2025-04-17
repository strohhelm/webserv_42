#include"../include/HttpRequest.hpp"



#include<unistd.h>
#include<stdio.h>


// PARENT                        CHILD
// ------                        -----
// |                            execve("php-cgi", ...)
// |                          
// |------ pipefdStdin[1] ---> stdin
// |                          
// stdout <--- pipefdStdout[1] --------|


void HttpRequest::runCgiScriptPost(const int& client_fd, const std::string& fullPath, const std::string& path)
{

	int pipefdStdout[2];
	int pipefdStdin[2];
	// pipe(pipefdStdout);  // for CGI output
	// pipe(pipefdStdin);   // for CGI input (POST data)

	if (pipe(pipefdStdout) == -1 || pipe(pipefdStdin) == -1 )
	{
		// close alll pipes before that!!!!!
		sendErrorResponse(client_fd, 500, "Pipe creation failed"); //???
		return;
	}
	
	pid_t pid = fork();
	if (pid < 0)
	{
		// close alll pipes before that!!!!!
		sendErrorResponse(client_fd, 500, "Fork failed"); //???
		return;
	}
	std::cout << BG_BRIGHT_MAGENTA << fullPath.c_str() << RESET << std::endl;
	if (pid == 0)
	{
		// CHILD: send stdout to pipe
		dup2(pipefdStdout[1], STDOUT_FILENO);
		dup2(pipefdStdin[0], STDIN_FILENO);
		close(pipefdStdout[0]); // we only write, don't read
		close(pipefdStdin[1]);  // we only read, don't write

		
	
		
		const char* phpCgiPath = "/usr/bin/php-cgi";
		char* argv[] =
		{
			(char*)phpCgiPath,
			(char*)fullPath.c_str(),
			NULL
		};
		
		
		auto it = _headers.find("Content-Length");
		if(it == _headers.end())
			sendErrorResponse(client_fd, 500, "Content-Lenght missing"); //???
		
		std::cout << BG_BRIGHT_RED << it->second << RESET << std::endl;

		std::vector<std::string> envStrings = {
			"REQUEST_METHOD=POST",
			"CONTENT_LENGTH=" + it->second, // from header
			"CONTENT_TYPE=application/x-www-form-urlencoded",
			"SCRIPT_FILENAME=" + fullPath,
			"SCRIPT_NAME=" + path,
			"GATEWAY_INTERFACE=CGI/1.1",
			"SERVER_PROTOCOL=HTTP/1.1",
			"REDIRECT_STATUS=200"

			// "SCRIPT_FILENAME=www/index.php",
			// "SCRIPT_NAME=/index.php",
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
		// close(pipefd[1]); // close write end

		std::string cgiOutput;
		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefdStdin[0], buffer, sizeof(buffer))) > 0)
		{
			cgiOutput.append(buffer, bytesRead);
		}
		// close(pipefd[0]);
	
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
		httpResponse += "Content-Type: text/html\r\n";
		httpResponse += "Content-Length: " + std::to_string(cgiOutput.size()) + "\r\n";
		httpResponse += "\r\n";
		httpResponse += cgiOutput;
	
		send(client_fd, httpResponse.c_str(), httpResponse.size(), 0);
	}
}
