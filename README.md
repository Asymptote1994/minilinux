# 写是最好的学——Linux学习记录

## 1.前言

### 1.1 学习Linux的痛点

因为自己平时的工作和linux相关，特别是驱动，所以一直想深入的学习下内核的其他子系统，比如进程管理、内存管理、文件系统等等。为此，也看过许多相关的书籍，文章，但是总感觉不能内化为自己的知识。

后面也想到学习类似“教你从零写操作系统”的课程，但是我发现有几点对于我来说不合适的地方：

1. 99%都是以x86架构来进行讲解，但是目前在嵌入式行业arm依旧是霸主地位
2. 虽然相较Linux简单很多，但是至少从代码的层面都是自己的一套，对于学习Linux帮助不是很直接

### 1.2 怎么学？

经过不断的尝试，通过这样的一种方式来学习Linux可能会更适合我：

1. 在一定的裸机代码的基础上（这个我下面会说明），从零、渐进地添加Linux代码进去，目标是先不管具体实现简单还是复杂，至少在功能上要和Linux无限接近
2. 使用arm架构的开发板进行实际的调试

基于上述原则，入手了一块正点原子的imx6ull的开发板，选择这个板子的原因也很简单，一是armv7单核处理器，二是该板子配套提供了能让芯片运行起来的裸机代码，这也是上面说的“在一定的裸机代码的基础上”的意思。

同时，我也会把自己的学习过程进行记录，一来能帮助自己对知识进行总结，加深理解，二来也可以供大家参考。

### 1.3 基础裸机代码

这里介绍下我使用的正点原子提供的裸机代码的目录结构，之后会在此基础上面进行更新：

~~~c
minilinux
├── bsp
│   ├── beep
│   ├── clk
│   ├── delay
│   ├── epittimer
│   ├── exit
│   ├── gpio
│   ├── int
│   ├── key
│   ├── keyfilter
│   ├── led
│   └── uart
├── imx6ul
├── obj
├── project
└── stdio
    ├── include
    └── lib
~~~

1. bsp：芯片的时钟、中断初始化以及各种外设的驱动等

2. imx6ul：芯片的寄存器定义头文件

3. obj：存放编译后产生的目标文件

4. project：启动代码，具体如下：

   ~~~c
   project
   ├── main.c
   └── start.S
   ~~~

5. stdio：提供printf等函数的实现代码

补充一句，工程名就叫minilinux吧！

### 1.4 后续更新的代码

后续我会在gitee上面进行更新，地址如下：

https://gitee.com/asymptotee/minilinux

## 2.从进程开始

进程是操作系统的核心，也是联系其他子系统的纽带。即使有Linux这个巨人，想要在代码层面从零实现进程管理并不简单。

那就先从最简单的进程切换开始，哦不对，应该是函数的切换。

### 2.1 在函数function1与function2之间连续切换

现在有如下两个函数：

```c
/*
 *	minilinux/project/main.c
 */
void function1(void)
{
	printf("enter %s\r\n", __func__);

	while (1) {
		printf("this is %s\r\n", __func__);
		delayms(200);
	}
}

void function2(void)
{
	printf("enter %s\r\n", __func__);

	while (1) {
		printf("this is %s\r\n", __func__);
		delayms(200);
	}
}
```

想要实现在两个函数之间连续切换，可以怎么做？

答：在function1中调用function2，在function2中调用function1

嗯....可以，但是似乎有点呆，并且不具备通用性。

还可以使用汇编语言：

~~~c
/*
 *	minilinux/project/start.S
 */
.global __switch_to
.type __switch_to function
__switch_to:
	mov pc, r0
~~~

这里将__switch_to声明为全局符号以及类型为function，具体的汇编代码只有一句mov pc, r0，这样可以在c代码中这样调用：

~~~c
__switch_to(function1);
~~~

因为第一个参数function1根据c与汇编的调用规则默认会传递给r0寄存器，而mov pc, r0的作用就是将r0的值给到pc寄存器，这样cpu可以直接跳转到pc，也就是function1的地址处，该地址也即是function1函数的起始地址。

修改后的main.c代码如下：

~~~c
/*
 *	minilinux/project/main.c
 */
void function1(void)
{
	printf("enter %s\r\n", __func__);

	while (1) {
		printf("this is %s\r\n", __func__);
		delayms(200);
        __switch_to(function2);
	}
}

void function2(void)
{
	printf("enter %s\r\n", __func__);

	while (1) {
		printf("this is %s\r\n", __func__);
		delayms(200);
        __switch_to(function1);
	}
}

int main(void)
{
	int_init(); 				/* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();			/* 初始化系统时钟 			*/
	delay_init();				/* 初始化延时 			*/
	clk_enable();				/* 使能所有的时钟 			*/
	led_init();					/* 初始化led 			*/
	beep_init();				/* 初始化beep	 		*/
	uart_init();				/* 初始化串口，波特率115200 */
	
	__switch_to(function1);

	return 0;
}
~~~

这样的话，便可以实现在两个函数之间连续切换，并且具备很好的通用性，因为__switch_to可以跳转到有效范围内任意给定的参数的位置。

执行情况如下：

~~~c
enter function1
this is function1                                                                              
enter function2                                                                                
this is function2                                                                              
enter function1                                                                                
this is function1                                                                              
enter function2                                                                                
this is function2                                                                              
...... 
~~~

嗯，看起来不错。

### 2.2 记录上次切换时的位置

仔细想想便可以发现虽然可以切换，但是每次切换后都会从函数的起始处开始执行，并不会在上次切换的位置处继续执行，这从打印的信息中也可以很容易发现。

这就已经可以借鉴Linux的代码了，也就是大名鼎鼎的__switch_to。这里我已经把和此处功能实现无关的代码删除，只留下了最核心的：

~~~c
/*
 *	linux/arch/arm/kernel/entry-armv.S
 */

/*
 * Register switch for ARMv3 and ARMv4 processors
 * r0 = previous task_struct, r1 = previous thread_info, r2 = next thread_info
 * previous and next are guaranteed not to be the same.
 */
ENTRY(__switch_to)
	add	ip, r1, #TI_CPU_SAVE
	stmia	ip!, {r4 - sl, fp, sp, lr}	@ Store most regs on stack
	add	r4, r2, #TI_CPU_SAVE
	ldmia	r4, {r4 - sl, fp, sp, pc}	@ Load all regs saved previously
ENDPROC(__switch_to)
~~~

对于arm架构的cpu，想要记住代码执行的位置，需要将切换时一些必要的cpu寄存器记录下来，当需要切换回来的时候再恢复到相应的cpu寄存器即可。

这样的话，针对每一个函数都需要一个结构体来将这些寄存器的值记录下来，在Linux中的arm架构下该结构体为：

~~~c
/*
 *	linux/arch/arm/include/asm/thread_info.h
 */
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
~~~

这里便可以引入著名的task_struct结构体，众所周知，该结构体用来记录一个进程的所有信息。但是cpu_context_save并不是直接内嵌在task_struct中，还有一个thread_info结构体：

~~~c
/*
 *	linux/arch/arm/include/asm/thread_info.h
 */
struct thread_info {
	struct cpu_context_save	cpu_context;	/* cpu context */
};

/*
 *	linux/include/linux/sched.h
 */
struct task_struct {
	struct thread_info	thread_info;
};
~~~

这里，关于这两个结构体我都只截取了目前用到的内容，后面会随着内容的增加进行添加。

关于__switch_to还有几点需要注意：__

1. ENTRY以及ENDPROC的实现见linux/include/linux/linkage.h，目前可以直接写出

2. 从注释可以看到__switch_to有三个参数，这里r0，也就是第一个参数目前用不到

3. TI_CPU_SAVE定义如下：

~~~c
/*
 *	linux/arch/arm/kernel/asm-offsets.c
 */
DEFINE(TI_CPU_SAVE,		offsetof(struct thread_info, cpu_context));
~~~

也就是cpu_context结构体在thread_info结构体的偏移，由于目前thread_info中只有cpu_context一个元素，所以可以设置为0即可。

综上，修改后的__switch_to如下：

~~~ c
/*
 *	linux/arch/arm/kernel/entry-armv.S
 */

/*
 * Register switch for ARMv3 and ARMv4 processors
 * r0 = previous task_struct, r1 = previous thread_info, r2 = next thread_info
 * previous and next are guaranteed not to be the same.
 */
.global __switch_to
.type __switch_to function
__switch_to:
	add	ip, r0, #0
	stmia	ip!, {r4 - sl, fp, sp, lr}	@ Store most regs on stack
	add	r4, r1, #0
	ldmia	r4, {r4 - sl, fp, sp, pc}	@ Load all regs saved previously
~~~

最后，main.c代码如下：

~~~c
/*
 *	minilinux/project/main.c
 */
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
~~~

需要注意的点：

1. 实例化两个task_struct结构体全局变量task1以及task2，并在main中对sp，也就是栈指针分别进行初始化；对pc，也就是cpu的跳转地址分别进行初始化
2. 在function1中调用__switch_to(&task1, &task2)，保存当前function1的执行位置到task1中的cpu_context，并恢复task2的cpu_context到cpu的寄存器，完成跳转
3. 在function2中调用__switch_to(&task2, &task1)，保存当前function2的执行位置到task2中的cpu_context，并恢复task1的cpu_context到cpu的寄存器，完成跳转
4. 由于修改__switch_to，所以将main中的__switch_to(function1)改为直接调用function1()

修改后执行情况如下：

~~~c
enter function1
this is function1                                                                              
function1(): before __switch_to                                                                
enter function2                                                                                
this is function2                                                                              
function2(): before __switch_to                                                                
function1(): after  __switch_to                                                                
this is function1                                                                              
function1(): before __switch_to                                                                
function2(): after  __switch_to                                                                
this is function2                                                                              
function2(): before __switch_to                                                                
function1(): after  __switch_to
......
~~~

可以看到，已经能记录到上次切换时执行的位置：

1. “enter function1”、“enter function2”分别只打印了一次
2. 以function1的切换过程为例，当function1在切换到function2又切换回来后，打印信息为“function1(): after  __switch_to”，也就是上次切换时的下一条指令的位置处



居然能在切换后在原位置继续执行，有点意思，不过这才是千里之行的第一步。但是缺点也很明显，需要显式的调用__switch_to来进行切换动作，可不可以借助一个外力来执行切换，而对于function们是无感的呢？

欲知后事如何，且听下回分解。







