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

static void pre_schedule_wrr(struct rq *rq, struct task_struct *prev)
{
	;
}

static void post_schedule_wrr(struct rq *rq)
{
	;
}
#endif /* CONFIG_SMP */


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
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	list_add_tail(&(p->wre), rq->wrr_rq);
	inc_nr_running(rq);
}

static void
dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	list_del(&p->wre);
	dec_nr_running(rq);
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_wrr_entity *wrr_se = &p->wre;
	
	update_curr_rt(rq);

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

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,
	.pre_schedule		= pre_schedule_wrr,
	.post_schedule		= post_schedule_wrr,
#endif

	.set_curr_task          = set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,
};
