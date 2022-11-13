/**************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 mian.c
作者	   : 左忠凯
版本	   : V1.0
描述	   : I.MX6U开发板裸机实验14 串口print实验
其他	   : 本实验在串口上移植printf，实现printf函数功能，方便以后的
		 程序调试。
论坛 	   : www.wtmembed.com
日志	   : 初版V1.0 2019/1/15 左忠凯创建
**************************************************************/
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_int.h"
#include "bsp_uart.h"
#include "stdio.h"

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
};

struct task_struct {
	struct thread_info		thread_info;
};

struct task_struct task1;
struct task_struct task2;

void function1(void);
void function2(void);

void function1(void)
{
	printf("enter %s\r\n", __func__);

	while (1) {
		printf("this is %s\r\n", __func__);
		delayms(200);
		printf("%s(): before __switch_to\r\n", __func__);
		__switch_to(&task1, &task2);
		printf("%s(): after  __switch_to\r\n", __func__);
	}
}

void function2(void)
{
	printf("enter %s\r\n", __func__);

	while (1) {
		printf("this is %s\r\n", __func__);
		delayms(200);
		printf("%s(): before __switch_to\r\n", __func__);
		__switch_to(&task2, &task1);
		printf("%s(): after  __switch_to\r\n", __func__);
	}
}

/*
 * @description	: main函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
	int_init(); 				/* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();			/* 初始化系统时钟 			*/
	delay_init();				/* 初始化延时 			*/
	clk_enable();				/* 使能所有的时钟 			*/
	led_init();					/* 初始化led 			*/
	beep_init();				/* 初始化beep	 		*/
	uart_init();				/* 初始化串口，波特率115200 */
	
	task1.thread_info.cpu_context.sp = 0x90000000;
	task2.thread_info.cpu_context.sp = 0x91000000;
	task1.thread_info.cpu_context.pc = function1;
	task2.thread_info.cpu_context.pc = function2;

	function1();

	return 0;
}
