#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <ctime>

const int TIMEOUT_MILLISECONDS = 10000;  // Timeout for inactivity (10 seconds)

void handleRequest(int fd) {
    std::cout << "Handling request on fd: " << fd << std::endl;
    // Simulate processing the request (e.g., reading data from the socket)
    // For now, we just print a message.
}

void closeClientConnection(int fd) {
    std::cout << "Timeout reached. Closing client connection on fd: " << fd << std::endl;
    close(fd);  // Close the client socket connection
}

int main() {
    // Simulate multiple server and client sockets
    std::vector<int> serverSockets = {0, 2, 4};  // Example server sockets
    std::vector<int> clientSockets = {1, 3, 5};  // Example client sockets

    // Create the pollfd structure to monitor file descriptors
    std::vector<struct pollfd> fds;
    
    // Map to track the last activity time for each client socket
    std::unordered_map<int, time_t> lastActivityTime;

    // Add server sockets to fds
    for (int serverSocket : serverSockets) {
        struct pollfd pfd;
        pfd.fd = serverSocket;
        pfd.events = POLLIN;  // Monitor for input (data to be read)
        fds.push_back(pfd);
    }

    // Add client sockets to fds and track the current time for each
    for (int clientSocket : clientSockets) {
        struct pollfd pfd;
        pfd.fd = clientSocket;
        pfd.events = POLLIN;  // Monitor for input (data to be read)
        fds.push_back(pfd);

        // Initialize the last activity time for each client
        lastActivityTime[clientSocket] = time(nullptr);
    }

    // Set up a loop to check for requests
    while (true) {
        // Poll for events with a timeoutpoll(
        int ret = fds.data(), fds.size(), TIMEOUT_MILLISECONDS);  // Timeout in milliseconds

        if (ret == -1) {
            std::cerr << "poll() failed: " << strerror(errno) << std::endl;
            break;
        }

        if (ret == 0) {
            // Timeout reached, check client inactivity
            std::cout << "Timeout reached. Checking client activity..." << std::endl;
            time_t currentTime = time(nullptr);

            // Loop through the client sockets and check for inactivity
            for (size_t i = 0; i < fds.size(); i++) {
                if (fds[i].fd % 2 != 0) {  // Only check client sockets (odd file descriptors)
                    time_t lastActivity = lastActivityTime[fds[i].fd];
                    double inactivityDuration = difftime(currentTime, lastActivity);

                    // If client has been inactive beyond the timeout threshold
                    if (inactivityDuration >= (TIMEOUT_MILLISECONDS / 1000)) {
                        closeClientConnection(fds[i].fd);  // Close client socket
                    }
                }
            }
        } else {
            // Check each file descriptor for events
            for (size_t i = 0; i < fds.size(); i++) {
                if (fds[i].revents & POLLIN) {  // If data is available to read
                    if (fds[i].fd % 2 == 0) {  // Server sockets have even fds
                        std::cout << "New request on server socket (fd " << fds[i].fd << ")" << std::endl;
                        handleRequest(fds[i].fd);  // Handle the request on server socket
                    } else {  // Client sockets
                        std::cout << "New request on client socket (fd " << fds[i].fd << ")" << std::endl;
                        handleRequest(fds[i].fd);  // Handle the request on client socket

                        // Update the last activity time for the client
                        lastActivityTime[fds[i].fd] = time(nullptr);
                    }
                }
            }
        }
    }

    return 0;
}
