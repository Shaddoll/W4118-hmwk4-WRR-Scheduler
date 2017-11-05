#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>


int main(int argc, char *argv[])
{
	fork();
	pid_t pid = getpid();
	int ret, count, count1, i;

	count = 0;
	count1 = 0;
	i = 0;
	struct sched_param param;
	param.sched_priority = 0;

	ret = sched_setscheduler(pid, 6, &param);
	if (ret != 0) {
		printf("error");
	}
	for(i=0;i<10;i++) {
		count = 0;
		while(count < 100000000) {
			count1++;
			count++;
		}
	}
	printf("%d", count1);
	return 0;
}
