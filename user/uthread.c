#include "uthread.h"
#include "user.h"

struct uthread *current_uthread;

int uthread_create(void (*start_func)(), enum sched_priority priority)
{
	struct uthread *iter_thread;
	int i = 0;
	for (iter_thread = uthread_table; iter_thread < &uthread_table[MAX_UTHREADS]; iter_thread++, i++)
	{
		if (iter_thread->state == FREE)
		{
			iter_thread->context.ra = (uint64) start_func;
			iter_thread->context.sp = (uint64) iter_thread->ustack;
			iter_thread->context.sp++;
			iter_thread->priority = priority;
			iter_thread->state = RUNNABLE;
			iter_thread->thread_index = i;
			return 0;
		}
	}
	return -1;
}

int getPriorityAsInt(enum sched_priority pr)
{
	return pr == LOW ? 0 : pr == MEDIUM ? 1
										: 2;
}

struct uthread *get_max_priority_uthread(struct uthread *my_thread)
{
	struct uthread *ret_thread = 0;
	int my_thread_index = my_thread->thread_index;
	my_thread_index = my_thread_index < -1 || my_thread_index > MAX_UTHREADS ? MAX_UTHREADS - 1 : my_thread_index;
	int i = (my_thread_index + 1) % MAX_UTHREADS;
	int max_priority = -1;

	int done = 0;
	while (!done)
	{
		struct uthread *t = &uthread_table[i];
		if ((t->state==RUNNABLE || t->state==RUNNING) && getPriorityAsInt(t->priority) > max_priority)
		{
			max_priority = getPriorityAsInt(t->priority);
			ret_thread = t;
		}
		if (i == my_thread_index)
			done = 1;
		i = (i + 1) % MAX_UTHREADS;
	}
	return ret_thread;
}

// TODO: maybe not working
void uthread_yield()
{
	struct uthread *my_thread = uthread_self();
	my_thread->state = RUNNABLE;
	struct uthread *next_to_run = get_max_priority_uthread(my_thread);
	uswtch(&my_thread->context, &next_to_run->context);
	// next_to_run->state = RUNNING;
}

void uthread_exit()
{
	struct uthread *my_thread = uthread_self();
	my_thread->state = FREE;
	int allFree = 1;
	for (struct uthread *iter_thread = uthread_table; iter_thread < &uthread_table[MAX_UTHREADS]; iter_thread++)
		if (iter_thread->state != FREE)
			allFree = 0;
	if (allFree)
		exit(0);
	struct uthread *next_to_run = get_max_priority_uthread(my_thread);
	uswtch(&my_thread->context, &next_to_run->context);
}

int uthread_start_all()
{
	// assuming there exists a thread which is runnable or running.
	if (started)
		return -1;
	started = 1;
	struct uthread *my_thread = &uthread_table[0];
	struct uthread *next_to_run = get_max_priority_uthread(my_thread);
	uswtch(&current_uthread->context, &next_to_run->context);
	return 0;
}

enum sched_priority uthread_set_priority(enum sched_priority priority)
{
	uthread_self()->priority = priority;
	return priority;
}

enum sched_priority uthread_get_priority()
{
	return uthread_self()->priority;
}

struct uthread *uthread_self()
{
	return current_uthread;
	// struct uthread *iter_thread;
	// for (iter_thread = uthread_table; iter_thread < &uthread_table[MAX_UTHREADS]; iter_thread++)
	// 	if (iter_thread->state == RUNNING)
	// 		return iter_thread;
	// return 0;
}
