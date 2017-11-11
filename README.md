Our goal of testing is to test each module of our scheduler, including enqueue and dequeue, load balancing and selecting the CPU with the minimum weight whenever a new task is to be allocated. Enqueue and dequeue operations can be tested through scheduler tick. 

We did a total of four experiments:

1. When there are multiple tasks on a run queue, we log the weight and the PID of the current task whenever the scheduler ticks. Using this method, we are able to monitor if the scheduler is indeed carrying out the round robin policy, and that the weight of the processes actually affects its runtime.

2. Selecting the CPU with the minimum weight is done by the select_task_rq_wrr function. We tested it by throwing 16 while(1) processes into the system 1 at a time. We can monitor each time which CPU the new process is allocated to. We saw that the CPU always choose to allocate the new task to an idle CPU if it exists, and no CPU ever had 3 processes in its run queue.

3. After forking 16 while processes, when we try to interact with the interface of the phone, we can only get one third of the runtime of the CPU. As a result, we try to increase the weight of processes whose user id is greater than 10000 so it would be allocated more time slots at the beginning. We tested to see if the lagging gets better when the weight increases.

4. Load balance is achieved by pulling tasks from other CPUs when a CPU becomes idle. This is implemented in wrr.c through the pull_task function. To test it, we changed the select_task_rq_wrr function so that it would allocate all tasks to a fixed CPU instead of finding the CPU with the minimum weight every time a task arrives. Then, we forked 10 while(1) processes to be run on that CPU. After a while, through our monitoring program, we can see that there are also processes running on each of the other CPUs.

