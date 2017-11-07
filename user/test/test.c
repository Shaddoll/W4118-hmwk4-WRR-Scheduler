#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "../../kernel/kernel/sched/wrr.h"

void print(struct wrr_info *wrr_info)
{
	int i;
	printf("=====================\n");
	for (i = 0; i < wrr_info->num_cpus; i++) {
		printf("\tcpu[%d]:\n", i);
		printf("\t\tnr_running:\t%d\n", wrr_info->nr_running[i]);
		printf("\t\ttotal_weight:\t%d\n", wrr_info->total_weight[i]);
		printf("-------------\n");
	}
}

int main(int argc, char *argv[])
{
	struct wrr_info u_wrr_info;
	int i = 0;
	printf("sdfasdfas\n");

	while (1) {
		syscall(244, &u_wrr_info);
		print(&u_wrr_info);
		sleep(3);
	}
	
	return 0;
}
