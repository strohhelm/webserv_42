#include <iostream>
#include <string>
#include <sstream>


void eraseSpaceAndTab(std::string &key, std::string &value)
{
	if (std::isspace(value.front()))
		value.erase(0, 1);
	if (std::isspace(value.back()))
		value.erase(value.length() - 1, 1);
	(void)key;
	// key.erase(key.find_last_not_of(" \t") + 1);
	// value.erase(0, value.find_first_not_of(" \t")); //idk what it does
}

int main (void)
{
	std::string lol("GET /reference/string/string/ HTTP/1.1 \r\n Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8 \r\nAccept-Encoding: gzip, deflate, br, zstd\r\nAccept-Language: en-US,en;q=0.9\r\nCache-Control: max-age=0\r\nConnection: keep-alive\r\nCookie: atp32=C%2B%2B11%7Cnull; vis31=00025dbc67ff89f1\r\nHost: cplusplus.com\r\nIf-None-Match: W/\"c12c-6BzmumcAEpr2nS4F9Q+H2sZ5A30\"\r\nReferer: https://cplusplus.com/\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nSec-GPC: 1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36\r\nsec-ch-ua: \"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Brave\";v=\"126\"\r\nsec-ch-ua-mobile: ?0\r\n sec-ch-ua-platform: \"macOS\"\r\n\r\n");
	size_t pos = lol.find("\r\n");
	std::string test = lol.substr(pos +2);
	
	// std::cout<<"\n"<<test<<std::endl;
	std::istringstream stream(test);

	while (std::getline(stream, test))
	{
		size_t delimiterPos = test.find(":");
		if(delimiterPos != std::string::npos)
		{
			std::string key = test.substr(0, delimiterPos);
			std::string value = test.substr(delimiterPos + 1);// cutting the ":"
			if (std::isspace(key[0]))
				continue;		//if a field line starts with whitespace, it is to be ignored or send 400 -> rcf 9112 page 7
			eraseSpaceAndTab(key, value); //TODO  see rules for parsing rcf9112 page 15 field syntax->link
			// _headers[key] = value;


			std::cout<<key<<"--|--"<<value<<std::endl;
		}
	}

}