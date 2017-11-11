Our goal of testing is to test each module of our scheduler, including enqueue, dequeue and requeue, load balancing and selecting the CPU with the minimum weight whenever a new task is to be allocated. Enqueue and dequeue operations can be tested through scheduler tick.

We did a total of four experiments:

1. When there are multiple tasks on a run queue, we log the weight and the PID of the current task whenever the scheduler ticks. Using this method, we are able to monitor if the scheduler is indeed carrying out the round robin policy, and that the weight of the processes actually affects its runtime. We were able to see that the tasks ran in a round robin manner.

Results:
Requeueing task: 9273 
Requeueing task: 9276
Requeueing task: 9278
Requeueing task: 9273 
Requeueing task: 9276
Requeueing task: 9278
Requeueing task: 9273 
Requeueing task: 9276
Requeueing task: 9278
…

2. Selecting the CPU with the minimum weight is done by the select_task_rq_wrr function. We tested it by throwing 16 while(1) processes into the system 1 at a time. We can monitor each time which CPU the new process is allocated to. We saw that the CPU always choose to allocate the new task to an idle CPU if it exists, and no CPU ever had 3 processes in its run queue.

Results:

4 X while(1):
=====================
	cpu[0]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[1]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[2]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[3]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[4]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[5]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[6]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[7]:
		nr_running:	1
		total_weight:	1
-------------


8 X while(1):
=====================
	cpu[0]:
		nr_running:	3
		total_weight:	3
-------------
	cpu[1]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[2]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[3]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[4]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[5]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[6]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[7]:
		nr_running:	1
		total_weight:	1
-------------


12 X while(1):
=====================
	cpu[0]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[1]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[2]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[3]:
		nr_running:	3
		total_weight:	3
-------------
	cpu[4]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[5]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[6]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[7]:
		nr_running:	1
		total_weight:	1
-------------


16 X while(1):
=====================
	cpu[0]:
		nr_running:	3
		total_weight:	3
-------------
	cpu[1]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[2]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[3]:
		nr_running:	3
		total_weight:	3
-------------
	cpu[4]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[5]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[6]:
		nr_running:	2
		total_weight:	2
-------------
	cpu[7]:
		nr_running:	3
		total_weight:	3
-------------



3. After forking 16 while processes, when we try to interact with the interface of the phone, we can only get one third of the runtime of the CPU. As a result, we try to increase the weight of processes whose user id is greater than 10000 so it would be allocated more time slots at the beginning. We tested to see if the lagging gets better when the weight increases. Surprisingly, the app is still very laggy. We then launched when long loops with the same work, and only set 1 to uid = 10000 while the user ids for the other processes are all less than 10000. We are able to observe that the process with the user id 10000 finished much faster.

4. Load balance is achieved by pulling tasks from other CPUs when a CPU becomes idle. This is implemented in wrr.c through the pull_task function. To test it, we changed the select_task_rq_wrr function so that it would allocate all tasks to a fixed CPU instead of finding the CPU with the minimum weight every time a task arrives. Then, we forked 10 while(1) processes to be run on that CPU. After a while, through our monitoring program, we can see that there are also processes running on each of the other CPUs.

Results:

	cpu[0]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[1]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[2]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[3]:
		nr_running:	11
		total_weight:	11
-------------
	cpu[4]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[5]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[6]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[7]:
		nr_running:	0
		total_weight:	0
-------------


	cpu[0]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[1]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[2]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[3]:
		nr_running:	14
		total_weight:	14
-------------
	cpu[4]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[5]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[6]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[7]:
		nr_running:	0
		total_weight:	0
-------------

	cpu[0]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[1]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[2]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[3]:
		nr_running:	7
		total_weight:	7
-------------
	cpu[4]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[5]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[6]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[7]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[0]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[1]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[2]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[3]:
		nr_running:	9
		total_weight:	9
-------------
	cpu[4]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[5]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[6]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[7]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[0]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[1]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[2]:
		nr_running:	0
		total_weight:	0
-------------
	cpu[3]:
		nr_running:	11
		total_weight:	11
-------------
	cpu[4]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[5]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[6]:
		nr_running:	1
		total_weight:	1
-------------
	cpu[7]:
		nr_running:	1
		total_weight:	1
-------------

