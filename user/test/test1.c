#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "../../kernel/kernel/sched/wrr.h"


int main(int argc, char *argv[])
{
	struct wrr_info u_wrr_info;
	int i = 0;
	
	setuid(10001);
	
	for (i = 0; i < 20; i++)
		if (fork())
			break;

	while (1) ;
	
	return 0;
}
