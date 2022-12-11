// #include "bsp_clk.h"
// #include "bsp_delay.h"
// #include "bsp_led.h"
// #include "bsp_beep.h"
// #include "bsp_key.h"
// #include "bsp_int.h"
// #include "bsp_uart.h"
// #include "stdio.h"


#include <clk.h>
#include <delay.h>
#include <led.h>
#include <int.h>
#include <uart.h>
#include <lib/stdio.h>

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

void print_task(void)
{
	printf("current_stack_pointer = 0x%x\r\n", current_stack_pointer);
	printf("current_thread_info   = 0x%x\r\n", current_thread_info());
	printf("current_task_struct   = 0x%x\r\n", current);
}

struct task_struct *task1 = 0x90000000;
struct task_struct *task2 = 0x91000000;

void function1(void);
void function2(void);

void function1(void)
{
	printf("################### enter %s ###################\r\n", __func__);

	while (1) {
		printf("++++++++ this is %s , step 1 ++++++++\r\n", __func__);
		delayms(200);
		printf("++++++++ this is %s , step 2 ++++++++\r\n", __func__);
		delayms(200);
		printf("++++++++ this is %s , step 3 ++++++++\r\n", __func__);
	}
}

void function2(void)
{
	printf("################### enter %s ###################\r\n", __func__);

	while (1) {
		printf("++++++++ this is %s , step 1 ++++++++\r\n", __func__);
		delayms(200);
		printf("++++++++ this is %s , step 2 ++++++++\r\n", __func__);
		delayms(200);
		printf("++++++++ this is %s , step 3 ++++++++\r\n", __func__);
	}
}

void do_work_pending(void)
{
	static unsigned char state = 1;

	state = !state;
	printf("============ this is %s, state = %d ============\r\n", __func__, state);

	if (state) {
		__switch_to(current, task1);
	} else {
		__switch_to(current, task2);
	}
}

int main(void)
{
	struct task_struct *unused_task;

	int_init(); 				/* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();			/* 初始化系统时钟 			*/
	delay_init();				/* 初始化延时 			*/
	clk_enable();				/* 使能所有的时钟 			*/
	led_init();					/* 初始化led 			*/
	uart_init();				/* 初始化串口，波特率115200 */
	epit1_init(0, 66000000 / 2);		/* 初始化EPIT1定时器，1分频，计数值为:66000000/2，也就是定时周期为500ms */                

	task1->thread_info.cpu_context.sp = (unsigned int)task1 + THREAD_SIZE;
	task2->thread_info.cpu_context.sp = (unsigned int)task2 + THREAD_SIZE;
	
	task1->thread_info.cpu_context.pc = function1;
	task2->thread_info.cpu_context.pc = function2;

	task1->thread_info.task = task1;
	task2->thread_info.task = task2;

	__switch_to(&unused_task, task1);

	return 0;
}
