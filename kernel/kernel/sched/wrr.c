#include "sched.h"

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
	return 0;
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
	
	result = list_first_entry(&((rq->wrr).queue), struct sched_wrr_entity, list);
	return result;
}

static void
enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
	struct list_head *queue = rq->wrr.queue;

	if (head)
		list_add(&wrr_se->list, queue);
	else
		list_add_tail(&wrr_se->list, queue);
	++rq->wrr.wrr_nr_running;
}

static void
dequeue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se)
{
	list_del_init(&wrr_se->list);
	--rq->wrr.wrr_nr_running;
}

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	INIT_LIST_HEAD(&wrr_rq->queue);
	wrr_rq->wrr_nr_running = 0;
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
	dequeue_wrr_entity(rq, wrr_se)
	dec_nr_running(rq);
}

/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_wrr_entity *wrr_se = &p->wre;
	struct list_head *queue = rq->wrr.queue;
	
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
	
	update_curr_wrr(rq);

	watchdog(rq, p);

	if (--p->wrr.time_slice)
		return;

	p->wrr.time_slice = WRR_TIMESLICE;
	
	if (wrr_se->list.prev != wrr_se->list.next) {
		requeue_task_wrr(rq, p, 0);
		set_tsk_need_resched(p);
	}
}

static void set_curr_task_wrr(struct rq *rq)
{
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
