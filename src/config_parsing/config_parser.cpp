#include "../../include/ServerConfig.hpp"

void getConfiguration(std::vector<ServerConfig> &Config)
{
	std::string filename = "../../config/nginx.conf";
	std::ifstream file (filename, std::fstream::in);

	if (! file.is_open())
	{
		throw "Error, couldnt open file: "<<filename<<std::endl;
		return 
	}
	
}
