#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "../../kernel/kernel/sched/wrr.h"


int main(int argc, char *argv[])
{
	syscall(245, 100);
	return 0;
}
