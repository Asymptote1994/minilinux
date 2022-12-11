#ifndef   __SCHED_H__
#define   __SCHED_H__

#include <list.h>

extern struct list_head task_head;

// 这里定义了进程运行可能处的状态。
#define TASK_RUNNING 0		// 进程正在运行或已准备就绪。
#define TASK_INTERRUPTIBLE 1	// 进程处于可中断等待状态。
#define TASK_UNINTERRUPTIBLE 2	// 进程处于不可中断等待状态，主要用于I/O 操作等待。
#define TASK_ZOMBIE 3		// 进程处于僵死状态，已经停止运行，但父进程还没发信号。
#define TASK_STOPPED 4		// 进程已停止。

#define __u32 unsigned int

struct cpu_context_save {
	__u32	r4;
	__u32	r5;
	__u32	r6;
	__u32	r7;
	__u32	r8;
	__u32	r9;
	__u32	sl;
	__u32	fp;
	__u32	sp;
	__u32	pc;
	__u32	extra[2];		/* Xscale 'acc' register, etc */
};

struct thread_info {
	struct cpu_context_save	cpu_context;	/* cpu context */
	struct task_struct	*task;		/* main task structure */
};

struct task_struct {
	struct thread_info		thread_info;
	struct list_head tasks;
	volatile long			state;
	long counter;
	long priority;
};

#define THREAD_SIZE			 4096

/*
 * how to get the current stack pointer in C
 */
register unsigned long current_stack_pointer asm ("sp");

/*
 * how to get the thread information struct from C
 */
// static inline struct thread_info *current_thread_info(void) __attribute_const__;
static struct thread_info *current_thread_info(void)
{
	return (struct thread_info *)
		(current_stack_pointer & ~(THREAD_SIZE - 1));
}

#define get_current() (current_thread_info()->task)
#define current get_current()

/*
 * switch_to(prev, next) should switch from task `prev' to `next'
 * `prev' will never be the same as `next'.  schedule() itself
 * contains the memory barrier to tell GCC not to cache `current'.
 */
extern struct task_struct *__switch_to(struct task_struct *, struct thread_info *, struct thread_info *);

static inline struct thread_info *task_thread_info(struct task_struct *task)
{
	return &task->thread_info;
}

#define switch_to(prev,next,last)					\
do {									\
	last = __switch_to(prev,task_thread_info(prev), task_thread_info(next));	\
} while (0)

#endif
