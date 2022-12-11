#include <sched.h>

static struct task_struct *alloc_task_struct_node(void)
{
	static int task_stack_addr_base = 0x90000000;

	struct task_struct *tsk = (struct task_struct *)task_stack_addr_base;
	task_stack_addr_base += THREAD_SIZE;

	return tsk;
}

int do_fork(void (*fn)(void))
{
	struct task_struct *tsk;

	tsk = alloc_task_struct_node();

	tsk->thread_info.cpu_context.pc = (__u32)fn;
	tsk->thread_info.cpu_context.sp = ((__u32)(tsk) + THREAD_SIZE);
	tsk->thread_info.task = tsk;

	tsk->counter = 15;
	tsk->priority = 15;
	tsk->state = TASK_RUNNING;

	list_add(&tsk->tasks, &task_head);

	return 0;
}

void print_task(void)
{
	printf("current_stack_pointer = 0x%x\r\n", current_stack_pointer);
	printf("current_thread_info   = 0x%x\r\n", current_thread_info());
	printf("current_task_struct   = 0x%x\r\n", current);
}
