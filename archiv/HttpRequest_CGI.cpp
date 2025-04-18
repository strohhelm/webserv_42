// #include"../include/ServerConfig.hpp"



// #include<unistd.h>
// #include<stdio.h>
// #include<array>



// #define CHILD 0

// void HttpRequest::runCgiScript(int& client_fd, const std::string& scriptPath, const std::string& method, const std::string& queryString, const std::string& postBody)


// const std::string	HttpRequest::getMethodString(void)
// {
// 	if(this->getMethod() == HttpMethod::GET)
// 		return "GET";
// 	if(this->getMethod() == HttpMethod::POST)
// 		return "POST";
// 	if(this->getMethod() == HttpMethod::DELETE)
// 		return "DELETE";
// 	return "UNKNOWN";
// }

// void HttpRequest::executeCGI(const int& client_fd, ServerConfig& config)
// {
//     std::string fullPath = config._rootDir + _requestLine._path;
//     std::string queryString;
//     std::string scriptPath;

//     size_t pos = fullPath.find('?');
//     if (pos != std::string::npos) {
//         scriptPath = fullPath.substr(0, pos); // "/index2.php"
//         queryString = fullPath.substr(pos + 1); // "name=Alice&lang=de"
//     }

//     int pipeParentToChild[2];
//     int pipeChildToParent[2];

//     // Init pipes and check return values
//     if (pipe(pipeParentToChild) == -1 || pipe(pipeChildToParent) == -1) {
//         sendErrorResponse(client_fd, 500, "Pipe creation failed");
//         return;
//     }

//     pid_t pid = fork();
//     if (pid < 0) {
//         sendErrorResponse(client_fd, 500, "Fork failed");
//         return;
//     }

//     // Child process
//     if (pid == 0) {
//         dup2(pipeParentToChild[0], STDIN_FILENO);    // set stdin = read-end of pipe
//         dup2(pipeChildToParent[1], STDOUT_FILENO);   // set stdout = write-end of pipe

//         // Close all unused pipe ends
//         close(pipeParentToChild[0]);
//         close(pipeParentToChild[1]);
//         close(pipeChildToParent[0]);
//         close(pipeChildToParent[1]);

//         const char* phpCgiPath = "/usr/bin/php-cgi";
//         char* argv[] = {
//             (char*)phpCgiPath,
//             (char*)fullPath.c_str(),
//             NULL
//         };

//         std::vector<std::string> envStrings = {
//             "GATEWAY_INTERFACE=CGI/1.1",
//             "REDIRECT_STATUS=200",  // required for php-cgi
//             "REQUEST_METHOD=" + getMethodString(),
//             "SCRIPT_FILENAME=" + fullPath,
//             "SCRIPT_NAME=" + fullPath,
//             "SERVER_PROTOCOL=HTTP/1.1",
//         };

//         if (getMethodString() == "GET") {
//             envStrings.push_back("QUERY_STRING=" + queryString);
//         }
//         else if (getMethodString() == "POST") {
//             envStrings.push_back("CONTENT_LENGTH=" + std::to_string(_rawBody.size()));
//             envStrings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
//         }

//         // Convert to char*[]
//         std::vector<char*> envp;
//         for (auto& s : envStrings)
//             envp.push_back(&s[0]);
//         envp.push_back(nullptr);

//         execve(phpCgiPath, argv, envp.data());
//         std::cerr << "execve failed: " << std::endl;
//         exit(1);
//     }
    
//     // Parent process
//     else {
//         close(pipeParentToChild[0]); // Parent doesn't need to read from this
//         close(pipeChildToParent[1]); // Parent doesn't need to write to this

//         // Send POST data if any
//         if (getMethodString() == "POST" && !_rawBody.empty()) {
//             write(pipeParentToChild[1], _rawBody.c_str(), _rawBody.size());
//         }
//         close(pipeParentToChild[1]); // done writing

//         // Read CGI output
//         std::string cgiOutput;
//         char buffer[1024];
//         ssize_t bytesRead;
//         while ((bytesRead = read(pipeChildToParent[0], buffer, sizeof(buffer))) > 0) {
//             cgiOutput.append(buffer, bytesRead);
//         }
//         close(pipeChildToParent[0]);

//         std::string httpResponse = "HTTP/1.1 200 OK\r\n";
//         httpResponse += "Content-Type: text/html\r\n";
//         httpResponse += "Content-Length: " + std::to_string(cgiOutput.size()) + "\r\n";
//         httpResponse += "\r\n";
//         httpResponse += cgiOutput;

//         send(client_fd, httpResponse.c_str(), httpResponse.size(), 0);
//     }
// }

