#ifndef WRR_H
#define WRR_H

#define MAX_CPUS	8
#define WRR_TIMESLICE	(10 * HZ / 1000)

extern void print_wrr_rq(struct seq_file *m, int cpu, struct wrr_rq *wrr_rq);

struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};


#endif
