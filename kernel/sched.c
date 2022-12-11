#include <sched.h>

struct list_head task_head;

struct task_struct *pick_next_task(void)
{
	int c;
	struct task_struct *temp_task;
	struct task_struct *next_task;

	while (1) {
		c = -1;

		list_for_each_entry(temp_task, &task_head, tasks) {
			if (temp_task->state == TASK_RUNNING && temp_task->counter > c) {
				c = temp_task->counter;
				next_task = temp_task;
			}
		}

		// 如果比较得出有counter 值大于0 的结果，则退出124 行开始的循环，执行任务切换（141 行）。
		if (c) {
			break;
		}

		// 否则就根据每个任务的优先权值，更新每一个任务的counter 值，然后回到125 行重新比较。
		// counter 值的计算方式为counter = counter /2 + priority。[右边counter=0??]		
		list_for_each_entry(temp_task, &task_head, tasks) {
			temp_task->counter = (temp_task->counter >> 1) + temp_task->priority;
		}
	}

	return next_task;
}

void schedule(void)
{
	struct task_struct *not_used_task;
	struct task_struct *next_task;

	next_task = pick_next_task();

	// // printf("######### %s(), before switch_to, next = %d #########\r\n", __func__, next);
	switch_to(current, next_task, not_used_task);
	// // printf("######### %s(), after switch_to #########\r\n", __func__);
}

void do_work_pending(void)
{
	schedule();
}
