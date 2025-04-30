#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/resource.h>
#include <filesystem>


int main (int argc, char** argv)
{
	// std::filesystem::path path = "./hallo/lol/pupu/index.php?haha=lol&pup=hih";
	// std::filesystem::path puth = "./hallo/lol/pupu/index";
	

	// std::cout<<path.extension()<<std::endl;
	// std::cout<<path.filename() <<std::endl;
	// std::cout<<path.has_relative_path() <<std::endl;

	// for (auto p:path)
	// 	std::cout<<p<<std::endl;
	
	// std::cout<<path.compare(puth);


	std::string request = "./hallo/lol/pupu/index.php?haha=lol&pup=hih";
		std::cout<< request.substr(request.find('?') + 1)<<std::endl;
	// std::string lol("GET /reference/string/string/ HTTP/1.1 \r\n Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8 \r\nAccept-Encoding: gzip, deflate, br, zstd\r\nAccept-Language: en-US,en;q=0.9\r\nCache-Control: max-age=0\r\nConnection: keep-alive\r\nCookie: atp32=C%2B%2B11%7Cnull; vis31=00025dbc67ff89f1\r\nHost: cplusplus.com\r\nIf-None-Match: W/\"c12c-6BzmumcAEpr2nS4F9Q+H2sZ5A30\"\r\nReferer: https://cplusplus.com/\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nSec-GPC: 1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36\r\nsec-ch-ua: \"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Brave\";v=\"126\"\r\nsec-ch-ua-mobile: ?0\r\n sec-ch-ua-platform: \"macOS\"\r\n\r\n");
	// size_t pos = lol.find("\r\n");
	// std::string test = lol.substr(pos +2);
	
	// // std::cout<<"\n"<<test<<std::endl;
	// std::istringstream stream(test);
	
	// while (std::getline(stream, test))
	// {
		// 	size_t delimiterPos = test.find(":");
		// 	if(delimiterPos != std::string::npos)
		// 	{
			// 		std::string key = test.substr(0, delimiterPos);
			// 		std::string value = test.substr(delimiterPos + 1);// cutting the ":"
			// 		if (std::isspace(key[0]))
			// 			continue;		//if a field line starts with whitespace, it is to be ignored or send 400 -> rcf 9112 page 7
			// 		eraseSpaceAndTab(key, value); //TODO  see rules for parsing rcf9112 page 15 field syntax->link
			// 		// _headers[key] = value;
			
			
			// 		std::cout<<key<<"--|--"<<value<<std::endl;
			// 	}
			// }
			
			// struct stat	fileinfo;
			// size_t		filesize;
			// if (stat(argv[1], &fileinfo) != 0)
			// 	exit(1);
			// // filesize = fileinfo.st_size;
			// // std::cout << "File: " << argv[1] << std::endl;
			// std::cout << "Device ID: " << fileinfo.st_dev << std::endl;
			// std::cout << "Inode number: " << fileinfo.st_ino << std::endl;
			// std::cout << "File mode: " << std::oct << fileinfo.st_mode << std::dec << std::endl;
			// std::cout << "Link count: " << fileinfo.st_nlink << std::endl;
			// std::cout << "User ID of owner: " << fileinfo.st_uid << std::endl;
			// std::cout << "Group ID of owner: " << fileinfo.st_gid << std::endl;
			// std::cout << "Device type (if inode device): " << fileinfo.st_rdev << std::endl;
			// std::cout << "Total size, in bytes: " << fileinfo.st_size << std::endl;
			// std::cout << "Blocksize for filesystem I/O: " << fileinfo.st_blksize << std::endl;
			// std::cout << "Number of blocks allocated: " << fileinfo.st_blocks << std::endl;
			// std::cout << "Last access time: " << ctime(&fileinfo.st_atime);
			// std::cout << "Last modification time: " << ctime(&fileinfo.st_mtime);
			// std::cout << "Last status change time: " << ctime(&fileinfo.st_ctime);
			
			
			// std::string path = "HttpRequest_Handler_Get.cpp";
			// std::cout<<path.substr(path.find_last_of('/') + 1)<<std::endl;
			
			
			
			
			
		}
		// void eraseSpaceAndTab(std::string &key, std::string &value)
		// {
			// if (std::isspace(value.front()))
			// 	value.erase(0, 1);
			// if (std::isspace(value.back()))
			// 	value.erase(value.length() - 1, 1);
			// (void)key;
			// key.erase(key.find_last_not_of(" \t") + 1);
			// value.erase(0, value.find_first_not_of(" \t")); //idk what it does
		// }