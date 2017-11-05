#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>


int main(int argc, char *argv[])
{
	pid_t pid = getpid();
	int ret, tt, i, j, k, count = 0;

	struct sched_param param;
	param.sched_priority = 0;

	ret = sched_setscheduler(pid, 6, &param);
	fork();
	while (1) ;
	
	return count;
}
