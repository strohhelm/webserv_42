#include <string>
#include <iostream>

int main(void)
{
	std::string path = "/var/www/html/index.html";
	while (!path.empty())
	{
		path = "hallloooooooo";
		std::cout<<path<<std::endl;
		size_t pos = path.find_last_of('/');
		if (pos == std::string::npos)
			break;
		path = path.substr(0, pos);
	}
}