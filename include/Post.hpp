#pragma once

#include "../include/HttpRequest.hpp"

struct userdata
{
	std::string username;
	std::string password;

};

class Post : public HttpRequest
{
  private:
	std::string path;
	std::string body;
	int fd;

  public:
	Post(std::string path, std::string body, std::string encoding, int fd);

	userdata urlDecode();
	void handleSignup();
	void handleLogin();
};