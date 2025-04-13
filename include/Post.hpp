# pragma once

#include"../include/HttpRequest.hpp"

class Post : public HttpRequest
{
    private:
        std::string path;
        std::string body;
        int fd;

    public:
        Post(std::string path, std::string body, std::string encoding, int fd);

        std::string urlDecode(const std::string &str);
        void handleSignup();
};