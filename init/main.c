#include <clk.h>
#include <delay.h>
#include <led.h>
#include <int.h>
#include <uart.h>
#include <lib/stdio.h>
#include <list.h>
#include <sched.h>

void function1(void)
{
	printf("################### enter %s ###################\r\n", __func__);

	while (1) {
		printf("++++++++ this is %s , step 1 ++++++++\r\n", __func__);
		delayms(200);
		printf("++++++++ this is %s , step 2 ++++++++\r\n", __func__);
		delayms(200);
		printf("++++++++ this is %s , step 3 ++++++++\r\n", __func__);
		delayms(200);
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
		delayms(200);
	}
}

void clean_bss(void)
{
	extern int __bss_start, __bss_end;
	int *p = &__bss_start;

	for (; p < &__bss_end; p++)
		*p = 0;
}

int main(void)
{
	struct task_struct *unused_task;

	clean_bss();

	int_init(); 				/* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();			/* 初始化系统时钟 			*/
	delay_init();				/* 初始化延时 			*/
	clk_enable();				/* 使能所有的时钟 			*/
	led_init();					/* 初始化led 			*/
	uart_init();				/* 初始化串口，波特率115200 */
	epit1_init(0, 66000000 / 2);		/* 初始化EPIT1定时器，1分频，计数值为:66000000/2，也就是定时周期为500ms */                

	INIT_LIST_HEAD(&task_head);

	do_fork(function1);
	do_fork(function2);

	struct task_struct *init_task = list_entry(task_head.next, struct task_struct, tasks);
	switch_to(unused_task, init_task, unused_task);

	return 0;
}
