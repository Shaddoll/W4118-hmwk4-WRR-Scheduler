#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>


int main(int argc, char *argv[])
{
	pid_t pid = getpid();
	int ret;

	struct sched_param param;
	param.sched_priority = 0;

	ret = sched_setscheduler(pid, 6, &param);
	if (ret != 0) {
		printf("error");
	}
	while(1);

	return 0;
}
