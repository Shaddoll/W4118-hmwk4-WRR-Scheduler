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
	struct task_struct *curr;
	struct rq *rq;

	int cpu, temp, result;
	int minimum_weight = 2147483647;
	
	rcu_read_lock();
	for_each_online_cpu(cpu) {
		temp = &cpu_rq(cpu)->wrr->total_weight;
		if (temp <= minimum_weight) {
			minimum_weight = temp;
			result = cpu;
		}
	}
	rcu_read_unlock();
	return result;
}

#endif /* CONFIG_SMP */

static void
update_curr_wrr(struct rq *rq)
{
	;
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
	//printk("=== pick next!\n");

	result = list_first_entry(&((rq->wrr).queue), struct sched_wrr_entity, list);

	p = container_of(result, struct task_struct, wre);
	return p;
}

static void
enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
	//printk("========= enqueue\n");
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
	//printk("========= dequeue\n");
	list_del_init(&wrr_se->list);
	rq->wrr.total_weight -= wrr_se->weight;
	--rq->wrr.wrr_nr_running;
}


void pull_wrr_task(int cpu_id) {
	int i, moved;
	struct rq *dest_rq = cpu_rq(cpu_id);
	struct rq *temp_rq;
	struct wrr_enitity *temp_wre;
	struct task_struct *p;

	moved = 0;
	for_each_online_cpu(i) {
		if (i == cpu_id)
			continue;

		temp_rq = cpu_rq(i);

		double_rq_lock(dest_rq, temp_rq);
		if (temp_rq->wrr_rq.nr_running == 1) {
			double_rq_unlock(dest_rq, temp_rq);
			continue;
		}

		list_for_each_entry(temp_wre, &((temp_rq->wrr_rq).queue), list) {			
			p = container_of(temp_wre, struct task_struct, wre);//get task struct

			if (p == temp_rq->curr)
				continue;

			if (!cpumask_test(cpu_id, p->cpus_allowed))
				continue;//check if task can work on current CPU

			dequeue_task_wrr(temp_rq, p, 0);
			enqueue_task_wrr(dest_rq, p, 0);//dequeue and enqueue
			moved = 1;
		}
		if (moved) {
			double_rq_unlock(dest_rq, temp_rq);
			return;
		}
		double_rq_unlock(dest_rq, temp_rq);
	}
}

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	INIT_LIST_HEAD(&wrr_rq->queue);
	wrr_rq->wrr_nr_running = 0;
	wrr_rq->total_weight = 0;
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

/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	printk("=== requeue\n");

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
}

static void watchdog(struct rq *rq, struct task_struct *p)
{
	;
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_wrr_entity *wrr_se = &p->wre;
	
	//return;
	
	update_curr_wrr(rq);

	watchdog(rq, p);
	
	printk("======= cpu: %d task_tick: %d time_slice: %d\n", cpu_of(rq), p->pid, p->wre.time_slice);

	if (--p->wre.time_slice)
		return;

	if (p->wre.weight > 1) {
		p->wre.weight = p->wre.weight - 1;
		--rq->wrr->total_weight;
	}
	p->wre.time_slice = WRR_TIMESLICE * p->wre.weight;

	if (wrr_se->list.prev != wrr_se->list.next) {
		requeue_task_wrr(rq, p, 0);
		set_tsk_need_resched(p);
	}
}

static void set_curr_task_wrr(struct rq *rq)
{
	struct task_struct *p = rq->curr;

	p->se.exec_start = rq->clock_task;
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
	;
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
	//.post_schedule		= post_schedule_wrr,
#endif

	.set_curr_task          = set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,
};
