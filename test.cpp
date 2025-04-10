#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cstdlib>

// Function to URL decode the values (basic implementation)
std::string urlDecode(const std::string &str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            // Convert hexadecimal representation to char
            int value;
            std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &value);
            result += static_cast<char>(value);
            i += 2;  // Skip the next two characters
        } else if (str[i] == '+') {
            result += ' ';  // Convert '+' to space
        } else {
            result += str[i];
        }
    }
    return result;
}

// Function to parse the request line (GET, POST, etc.)
void parseRequestLine(const std::string &requestLine, std::string &method, std::string &path, std::string &protocol) {
    std::istringstream stream(requestLine);
    stream >> method >> path >> protocol;
}

// Function to parse headers
std::unordered_map<std::string, std::string> parseHeaders(const std::string &headers) {
    std::unordered_map<std::string, std::string> headerMap;
    std::istringstream stream(headers);
    std::string line;
    
    while (std::getline(stream, line)) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            // Trim spaces from key and value
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            headerMap[key] = value;
        }
    }
    return headerMap;
}

// Function to parse the body (URL-encoded form data)
std::unordered_map<std::string, std::string> parsePostData(const std::string &data) {
    std::unordered_map<std::string, std::string> parsedData;
    size_t delimiterPos = data.find('=');
    
    if (delimiterPos != std::string::npos) {
        std::string key = data.substr(0, delimiterPos);
        std::string value = data.substr(delimiterPos + 1);

        // URL decode the key and value
        key = urlDecode(key);
        value = urlDecode(value);

        // Store the key-value pair in the map
        parsedData[key] = value;
    }

    return parsedData;
}

// Function to parse the entire HTTP POST request
void parseHttpRequest(const std::string &request, std::string &method, std::string &path, std::string &protocol,
                      std::unordered_map<std::string, std::string> &headers, std::unordered_map<std::string, std::string> &body) {
    std::istringstream stream(request);
    std::string line;

    // Parse the request line (GET, POST, etc.)
    std::getline(stream, line);
    parseRequestLine(line, method, path, protocol);

    // Parse headers (ends when an empty line is found)
    std::string headerData;
    while (std::getline(stream, line) && !line.empty()) {
        headerData += line + "\n";
    }
    headers = parseHeaders(headerData);

    // The body (after headers) is the remaining data
    std::string bodyData;
    while (std::getline(stream, line)) {
        bodyData += line;
    }

    // Parse the POST data (key-value pairs)
    body = parsePostData(bodyData);
}

int main() {
    // Simulating the given POST request
    std::string request = 
        "POST / HTTP/1.1\n"
        "Host: localhost:8080\n"
        "Connection: keep-alive\n"
        "Content-Length: 17\n"
        "Cache-Control: max-age=0\n"
        "sec-ch-ua: \"Chromium\";v=\"134\", \"Not:A-Brand\";v=\"24\", \"Google Chrome\";v=\"134\"\n"
        "sec-ch-ua-mobile: ?0\n"
        "sec-ch-ua-platform: \"Windows\"\n"
        "Origin: http://localhost:8080\n"
        "Content-Type: application/x-www-form-urlencoded\n"
        "Upgrade-Insecure-Requests: 1\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\n"
        "Sec-Fetch-Site: same-origin\n"
        "Sec-Fetch-Mode: navigate\n"
        "Sec-Fetch-User: ?1\n"
        "Sec-Fetch-Dest: document\n"
        "Referer: http://localhost:8080/\n"
        "Accept-Encoding: gzip, deflate, br, zstd\n"
        "Accept-Language: de-DE,de;q=0.9,en-GB;q=0.8,en;q=0.7,en-US;q=0.6\n"
        "\n"
        "message=testttttt";

    // Variables to store parsed parts
    std::string method, path, protocol;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> body;

    // Parse the entire HTTP request
    parseHttpRequest(request, method, path, protocol, headers, body);

    // Output parsed data
    std::cout << "Method: " << method << "\n";
    std::cout << "Path: " << path << "\n";
    std::cout << "Protocol: " << protocol << "\n";
    
    std::cout << "\nHeaders:\n";
    for (const auto& [key, value] : headers) {
        std::cout << key << ": " << value << "\n";
    }

    std::cout << "\nBody:\n";
    for (const auto& [key, value] : body) {
        std::cout << key << ": " << value << "\n";
    }

    return 0;
}



void HttpRequest::handleGet(int server_fd, int client_fd)
{
    // Determine the server config based on the accepted connection's server_fd (port)
    if (serverConfigs.find(server_fd) == serverConfigs.end()) {
        sendErrorResponse(client_fd, 404, "404 Not Found");
        return;
    }
    
    // Retrieve the server configuration for this server_fd
    ServerConfig config = serverConfigs[server_fd];
    std::string serverName = config.server_name;
    std::string rootDirectory = config.root_directory;

    // Now handle the request dynamically based on the server configuration
    std::string content;
    bool isFile = true;
    std::string path = getRequestedFile(isFile); // Get the requested file based on path from HTTP request

    if (path.empty()) {
        sendErrorResponse(client_fd, 403, "403 Forbidden");
        return;
    }

    // Make sure to prepend the root directory for the requested file
    path = rootDirectory + path;

    if (isFile) {
        content = readFileContent(path);
    } else {
        content = path;  // If it's not a file, just return the path as a response
    }

    if (content.empty()) {
        sendErrorResponse(client_fd, 404, "404 Not Found");
        return;
    }

    sendResponse(client_fd, 200, content);
}
