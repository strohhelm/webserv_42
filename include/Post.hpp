#pragma once

#include "../include/HttpRequest.hpp"

#include <filesystem>
#include <cstdio>

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
	std::filesystem::path _path;

	bool _done = false;

	RequestState &_state;

  public:
	Post(std::string path, std::string body, std::string encoding, const int &fd, RequestState &_state);


	void handleUpload();
	int extractInfo();
	int dirSetup();
	int extractContent();
	int writeContent();
	void postRespond();
	void checkFilename(std::filesystem::path filePath);
	// size_t findCheck(std::string hay, char needle, size_t pos);
	// size_t findCheck(std::string hay, std::string needle, size_t pos);

};
