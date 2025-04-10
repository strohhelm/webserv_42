#include"../include/HttpRequest.hpp"



void HttpRequest::handleHttpRequest(int client_fd, int server_fd, ServerConfig config)
{
	switch (getMethod())
	{
		case HttpMethod::GET:
			handleGet(client_fd, server_fd, config);
			break;
		case HttpMethod::POST:
			handlePost(client_fd);
			break;
		case HttpMethod::DELETE:
			handleDelete(client_fd);
			break;		
		default:
			handleUnknown(client_fd);
			break;
	}
}

#include<unistd.h>
#include<stdio.h>


void HttpRequest::runCgiScript(int& client_fd, const std::string& scriptPath)
{
	std::string scriptPath2 =  "/src" + scriptPath;
	
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        sendErrorResponse(client_fd, 500, "Pipe creation failed");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        sendErrorResponse(client_fd, 500, "Fork failed");
        return;
    }

	std::cout << BG_BRIGHT_MAGENTA << scriptPath2.c_str() << RESET << std::endl;
    if (pid == 0) {
        // CHILD: send stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

		char* argv[] = { (char*)scriptPath2.c_str(), NULL };
		char* envp[] = { NULL }; // or fill with CGI env
	
		execve(scriptPath2.c_str(), argv, envp);
		exit(1); // only reached if exec fails
        // execl(scriptPath2.c_str(), scriptPath2.c_str(), NULL);

        // // if execl fails
		// std::cout << "exel failed" << std::endl;
        // perror("execl");
        // exit(1);
    } else {
        // PARENT: read from pipe
        // close(pipefd[1]);

        // std::string output;
        // char buffer[1024];
        // ssize_t bytesRead;
        // while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        //     output.append(buffer, bytesRead);
        // }
        // close(pipefd[0]);

        // if (output.empty()) {
        //     sendErrorResponse(client_fd, 500, "CGI script produced no output");
        //     return;
        // }

        // send(client_fd, output.c_str(), output.size(), 0);

		close(pipefd[1]); // close write end

		std::string cgiOutput;
		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
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



void HttpRequest::handleGet(int client_fd, int server_fd, ServerConfig config)
{
	// If Host is missing in an HTTP/1.1 request, return 400 Bad Request.
	bool isFile = true;
	std::string	content;
	std::string path = getRequestedFile(isFile, config);
	(void)server_fd;

	if(_requestLine._path == "/cgi")
	{	
		
		std::string scriptPath = "/cgi-bin/test.cgi";
		runCgiScript(client_fd, scriptPath);

		// testCGI(client_fd);
		return;
	}

	


	std::cout << BG_BRIGHT_BLUE << config.getRootDir() << RESET << std::endl;
	// std::cout << BG_BRIGHT_BLUE << "path " << path << RESET << std::endl;
	if(path.empty())
	{
		sendErrorResponse(client_fd, 404, "404 Not Found");
		return;
	}
	if(isFile)
	{
		content = readFileContent(path);
	}
	else
	{
		content = path;
	}
	
	// std::cout << BG_BRIGHT_BLUE << "content [" << content << "]" << RESET << std::endl;
	
	if(content.empty())
	{
		sendErrorResponse(client_fd, 403, "403 Forbidden");
		return;
	}
	sendResponse(client_fd, 200, content);	
}

std::string HttpRequest::getContentType()
{
	auto type = _headers.find("Content-Type");
	
	if(type == _headers.end())
	{
		return "";
	}
	else
	{
		std::cout << BG_BRIGHT_BLACK << type->second << RESET << std::endl;
		return type->second;
	}
}




void HttpRequest::handlePost(int fd)
{
	// If Content-Length is missing for a POST request, return 411 Length Required.
	// If Content-Length does not match the actual body size, return 400 Bad Request

	// handleGet(fd);
	
	// pathToCGI
	auto it = _body.find("CGI");
	// std::cout << it.first << std::endl;	

	// if(getContentType() != "")
	if(it->first == "CGI")
	{
		// runCgiScript(fd, "cgi-bin");
		// sendErrorResponse(fd, 405, "405 CGI   " + it->second);// wrong Code 
	}
	else
		sendErrorResponse(fd, 405, "405 NO CGI");// wrong Code 
	
}

void HttpRequest::handleUnknown(int fd)
{
	sendErrorResponse(fd, 405, "405 Method Not Allowed");
}





int HttpRequest::deleteFile(const std::string& filename)
{
	return (remove(filename.c_str()) == 0);
}



void HttpRequest::handleDelete(int fd)
{
	std::string path = getPath();
	if (path.front() == '/') {
		path.erase(path.begin());  // Remove the leading "/"
	}
	std::cout << BG_BRIGHT_BLACK << path << RESET << std::endl;

	if(!fileExists(path))
	{
		sendErrorResponse(fd, 404, "404 Not Found");
		return;		
	}
	if(!deleteFile(path))
	{
		sendErrorResponse(fd, 500, "500 Failed to delete resource");
		return;
	}
	sendResponse(fd,204, "Resource deleted successfully");


}
