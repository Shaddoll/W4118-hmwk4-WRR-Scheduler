#include "sched.h"
#include "wrr.h"




/*
 * wrr-task scheduling class.
 *
 * (NOTE: these are not related to SCHED_wrr tasks which are
 *  handled in sched/fair.c)
 */


#ifdef CONFIG_SMP


static int
select_task_rq_wrr(struct task_struct *p, int sd_flag, int flags)
{
	int cpu, temp, result;
	int minimum_weight;

	//return 3;
	
	result = task_cpu(p);
	if (p->nr_cpus_allowed == 1)
		return result;
	
	minimum_weight = cpu_rq(result)->wrr.total_weight;

	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (!cpumask_test_cpu(cpu, tsk_cpus_allowed(p)))
			continue;
		temp = cpu_rq(cpu)->wrr.total_weight;
		//printk("cpu: %d, weight: %d\n", cpu, temp);
		if (temp < minimum_weight) {
			minimum_weight = temp;
			result = cpu;
		}
	}
	rcu_read_unlock();
	//printk("choose cpu: %d, weight: %d\n", result, minimum_weight);
	return result;
}

#endif /* CONFIG_SMP */

static void
update_curr_wrr(struct rq *rq)
{

	struct task_struct *curr = rq->curr;
	u64 delta_exec;

	if (curr->sched_class != &wrr_sched_class)
		return;

	delta_exec = rq->clock_task - curr->se.exec_start;
	if (unlikely((s64)delta_exec <= 0))
		return;

	schedstat_set(curr->se.statistics.exec_max,
		      max(curr->se.statistics.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	account_group_exec_runtime(curr, delta_exec);

	curr->se.exec_start = rq->clock_task;
	cpuacct_charge(curr, delta_exec);

}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	return;
}

static struct task_struct *pick_next_task_wrr(struct rq *rq)
{
	struct sched_wrr_entity *result;
	struct task_struct *p;
	
	
	if (rq->wrr.wrr_nr_running == 0)
		return NULL;

	result = list_first_entry(&((rq->wrr).queue), struct sched_wrr_entity, list);

	p = container_of(result, struct task_struct, wre);
	return p;
}

static void
enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
	struct list_head *queue = &(rq->wrr.queue);

	if (head)
		list_add(&wrr_se->list, queue);
	else
		list_add_tail(&wrr_se->list, queue);
	rq->wrr.total_weight += wrr_se->weight;
	++rq->wrr.wrr_nr_running;
}

static void
dequeue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se)
{
	list_del_init(&wrr_se->list);
	rq->wrr.total_weight -= wrr_se->weight;
	--rq->wrr.wrr_nr_running;
}
static void
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wre;

	enqueue_wrr_entity(rq, wrr_se, flags & ENQUEUE_HEAD);	
	inc_nr_running(rq);
}

static void
dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wre;

	update_curr_wrr(rq);
	dequeue_wrr_entity(rq, wrr_se);
	dec_nr_running(rq);
}

void pull_wrr_task(int dst_cpu) {
	int src_cpu;
	struct rq *dst_rq = cpu_rq(dst_cpu);
	struct rq *src_rq;
	struct sched_wrr_entity *src_wre;
	struct task_struct *p;
	
	//return;
	
	//printk("============= cpu %d try to pull!\n", dst_cpu);
	if (!cpu_online(dst_cpu)) {
		//printk("========== cpu %d not online!\n", dst_cpu);
		return;
	}
	
	src_rq = NULL;
	for_each_online_cpu(src_cpu) {
		if (src_cpu == dst_cpu)
			continue;

		src_rq = cpu_rq(src_cpu);

		double_rq_lock(dst_rq, src_rq);
		if (list_empty(&src_rq->wrr.queue)) {
			double_rq_unlock(dst_rq, src_rq);
			//if (src_cpu == 3)
				//printk("!!!!!!!!!!!!!!!!!!!!!!!! cpu 3 empty!\n");
			continue;
		}

		list_for_each_entry(src_wre, &src_rq->wrr.queue, list) {			
			p = container_of(src_wre, struct task_struct, wre);//get task struct

			if (task_running(src_rq, p)) {
				//if (src_cpu == 3)
					//printk("!!!!!!!!!!!!!!!!!!!!!!!! cpu 3 is running p!\n");
				continue;
			}
				
			if (p->policy != SCHED_WRR) {
				//if (src_cpu == 3)
					//printk("!!!!!!!!!!!!!!!!!!!!!!!! p on cpu 3 is not WRR!\n");
				continue;
			}

			if (!cpumask_test_cpu(dst_cpu, tsk_cpus_allowed(p))) {
				//if (src_cpu == 3)
					//printk("!!!!!!!!!!!!!!!!!!!!!!!! p on cpu 3 does not want to be run on cpu %d!\n", dst_cpu);
				continue;//check if task can work on current CPU
			}
			
			if (p->on_rq) {
				
				deactivate_task(src_rq, p, 0);
				set_task_cpu(p, dst_cpu);
				activate_task(dst_rq, p, 0);//dequeue and enqueue
				
				printk("==== steal from %d to %d \n", src_cpu, dst_cpu);
				
				check_preempt_curr(dst_rq, p, 0);
				
				double_rq_unlock(dst_rq, src_rq);
				return;
			}
			//else {
			//	if (src_cpu == 3)
			//		printk("!!!!!!!!!!!!!!!!!!!!!!!! p on cpu 3 is not on rq!\n");
			//}
			
		}

		double_rq_unlock(dst_rq, src_rq);
	}
}

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	INIT_LIST_HEAD(&wrr_rq->queue);
	wrr_rq->wrr_nr_running = 0;
	wrr_rq->total_weight = 0;
}


/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_wrr_entity *wrr_se = &p->wre;
	struct list_head *queue = &(rq->wrr.queue);

	if (head)
		list_move(&wrr_se->list, queue);
	else
		list_move_tail(&wrr_se->list, queue);
}

static void
yield_task_wrr(struct rq *rq)
{
	requeue_task_wrr(rq, rq->curr, 0);
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
	update_curr_wrr(rq);
	prev->se.exec_start = 0;
}

static void watchdog(struct rq *rq, struct task_struct *p)
{

}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_wrr_entity *wrr_se = &p->wre;

	update_curr_wrr(rq);

	watchdog(rq, p);
	
	//printk("======= cpu: %d task_tick: %d time_slice: %d\n", cpu_of(rq), p->pid, p->wre.time_slice);

	if (--p->wre.time_slice > 0)
		return;

	if (p->wre.weight > 1) {
		p->wre.weight = p->wre.weight - 1;
		--rq->wrr.total_weight;
	}
	p->wre.time_slice = WRR_TIMESLICE * p->wre.weight;

	if (wrr_se->list.prev != wrr_se->list.next) {
		requeue_task_wrr(rq, p, 0);
		resched_task(p);
	}
}

static void set_curr_task_wrr(struct rq *rq)
{
	struct task_struct *p = rq->curr;

	p->se.exec_start = rq->clock_task;
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
	if (p->on_rq && rq->curr != p)
		if (rq == task_rq(p) && !rt_task(rq->curr))
			resched_task(rq->curr);
}

static void
prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
	;
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	return WRR_TIMESLICE;
}

/*
 * Simple, special scheduling class for the per-CPU wrr tasks:
 */
const struct sched_class wrr_sched_class = {
	.next 			= &fair_sched_class,
	.enqueue_task 		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,
	//.pre_schedule		= pre_schedule_wrr,
	//.post_schedule	= post_schedule_wrr,
#endif

	.set_curr_task          = set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,
};

#ifdef CONFIG_SCHED_DEBUG
extern void print_wrr_rq(struct seq_file *m, int cpu, struct wrr_rq *wrr_rq);

void print_wrr_stats(struct seq_file *m, int cpu)
{
	struct wrr_rq *wrr_rq;

	rcu_read_lock();
	wrr_rq = &(cpu_rq(cpu)->wrr);
	print_wrr_rq(m, cpu, wrr_rq);
	rcu_read_unlock();
}
#endif /* CONFIG_SCHED_DEBUG */
