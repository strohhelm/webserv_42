#pragma once

#include "../include/HttpRequest.hpp"

#include <filesystem>
#include <cstdio>

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
	std::string _contentHeader;
	const int &fd;

	std::string _fileContent;

	std::string _uploadDir = "file_upload";
	std::string _tempDir = "tmp_upload";
	std::string _fdPath;

	bool _done = false;

	RequestState &_state;

  public:
	Post(std::string path, std::string body, std::string encoding, const int &fd, RequestState &_state);

	// userdata urlDecode();
	// void handleSignup();
	// void handleLogin();
	void handleUpload();
	void extractInfo();
	void dirSetup();
	void extractContent();
	void writeContent();
	void postRespond();
	size_t findCheck(std::string hay, char needle, size_t pos);
	size_t findCheck(std::string hay, std::string needle, size_t pos);

};