#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>


int main(int argc, char *argv[])
{
	int i = 0;
	
	setuid(10001);
	
	//for (i = 0; i < 2; i++)
	//	if (fork())
	//		break;

	while (1) ;
	
	return 0;
}
