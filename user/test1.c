#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>


int main(int argc, char *argv[])
{
	//int i = 0;
	
	setuid(20000);
	
	printf("%d\n", getpid());
	
	//for (i = 0; i < 2; i++)
	//	if (fork())
	//		break;

	while (1) {
		//printf("block\n");
		;
	}
	
	return 0;
}
