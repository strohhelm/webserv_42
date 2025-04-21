#include "../include/SimpleServer.hpp"

#include "../include/SignalHandler.hpp"





void SimpleServer::resetIdleTimeout(int fdIndex)
{
    int client_fd = _poll_fds[fdIndex].fd;
    
    // Record the current time as the last activity time for the client
    _clientLastActivityTimes[client_fd] = std::chrono::steady_clock::now();

    std::cout << "Idle timeout for client " << client_fd << " reset." << std::endl;
}

bool SimpleServer::shouldCloseConnection(int fdIndex)
{
    int client_fd = _poll_fds[fdIndex].fd;

    // Check for any protocol-specific conditions (e.g., HTTP request processing)
    // If using HTTP, you might close the connection after the request is fully processed
    // Example: If you've handled the HTTP request and don't expect further data

    // Here we return false if the connection should remain open
    // For simplicity, we'll check the following conditions:

    // 1. If the client closed the connection
    if (_recvBuffer.find(client_fd) != _recvBuffer.end() && _recvBuffer[client_fd].empty()) {
        std::cout << "Client " << client_fd << " has closed the connection." << std::endl;
        return true;  // Should close the connection
    }

    // 2. If there was an error with the connection (error detected during recv)
    // This check can be triggered in your readDataFromClient function after receiving -1 from recv()
    // if (_errorDetected[client_fd]) {
    //     std::cerr << "Error detected on client " << client_fd << ". Closing connection." << std::endl;
    //     return true;  // Should close the connection
    // }

    // 3. Optional: Timeout condition (this requires tracking time since last activity)
    // If you have some timeout logic in your server (e.g., inactivity timeout), you can check it here.
    // if (hasTimedOut(client_fd)) {
    //     std::cout << "Connection timed out for client " << client_fd << "." << std::endl;
    //     return true;  // Should close the connection
    // }

    // You can add more protocol-specific checks here.

    // If none of the conditions are met, the connection should remain open
    return false;
}


void SimpleServer::closeConnection(int fdIndex)
{
    int client_fd = _poll_fds[fdIndex].fd;

    // Close the client socket
    if (close(client_fd) < 0)
    {
        std::cerr << RED << "Failed to close client socket " << client_fd << ": " << strerror(errno) << RESET << std::endl;
    }
    else
    {
        std::cout << GREEN << "Client socket " << client_fd << " closed successfully." << RESET << std::endl;
    }

    // Remove the client from the poll_fds vector
    _poll_fds.erase(_poll_fds.begin() + fdIndex);

    // Also, remove the client's buffer from _recvBuffer if needed
    _recvBuffer.erase(client_fd);
}

