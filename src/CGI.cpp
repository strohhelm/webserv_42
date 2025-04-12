#include"../include/CGI.hpp"



/*
fd[2][2];
fd[x][0] = read
fd[x][1] = write
*/

/*
#define STDIN_FILENO  0  // standard input
#define STDOUT_FILENO 1  // standard output
#define STDERR_FILENO 2  // standard error
*/


void CGI::closePipesFromFd(std::array<int, 2>& fd)
{
	close(fd[READ_FD]);
	close(fd[WRITE_FD]);
}

void	CGI::closeAllPipes(void)
{
	close(_parent[READ_FD]);
	close(_parent[WRITE_FD]);
	close(_child[READ_FD]);
	close(_child[WRITE_FD]);	
}


int	CGI::createPipes()
{
	if(pipe(_parent.data()) == -1)
	{
		return 1;
	}
	if(pipe(_child.data()) == -1)
	{
		closePipesFromFd(_parent);
		return 1;
	}
	return 0;
}


void CGI::handleChildProcess(void)
{

}

void CGI::handleParentProcess(void)
{

}

// std::array<int, 3> a = {1, 2, 3};
void CGI::execute(void)
{
	if(createPipes())
	{
		// sendErrorResponse(client_fd, 500, "Pipe creation failed"); //???
		// return
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		closeAllPipes();
		// sendErrorResponse(client_fd, 500, "Fork failed");
		return;
	}

	if(pid == 0)
	{
		handleChildProcess();
	}
	else
	{
		handleParentProcess();
	}
	closeAllPipes();
}