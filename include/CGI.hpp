

#ifndef CGI_HPP
# define CGI_HPP



#include<unistd.h>
#include<stdio.h>
#include<array>


#define READ_FD 0
#define WRITE_FD 1

class CGI
{
	private:
		std::array<int, 2> _parent;
		std::array<int, 2> _child;
		char** _argv;
		char* _phpCgiPath;

	public:
		void	closePipesFromFd(std::array<int, 2>& fd);
		void	closeAllPipes(void);
		int		createPipes(void);
		void	handleChildProcess(void);
		void	handleParentProcess(void);
		void	execute(void);

		void	setPipeToRead(int fd);
		void	setPipeToWrite(int fd);
		void	setArgv(void);


};


#endif