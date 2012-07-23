/* filename mem_test.c
 *主测试模块.同时调用另一个模块中的函数 
 */
#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/delay.h>
//#include <linuxst.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <linux/kernel.h> /* printk() */
#include <linux/fs.h> /* everything... */
#include <linux/kthread.h>
//#include <linuxev.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-irq.h>
#include <asm/uaccess.h> /* copy_to_user() */
#include <linux/delay.h>	/* mdelay() */
#include <linux/irq.h>

//外部定义函数 ./api/kgen_plat.c z中
//kmalloc系列
extern int test_func_api(void);
extern char *kgen_malloc(unsigned int iLen);
extern char *kgen_malloc_dma(unsigned int iLen);
extern void kgen_free(void *ptr);
extern char *kgen_malloc_pages(unsigned int iLen);
extern char *kgen_malloc_dma_pages(unsigned int iLen);
extern void kgen_free_pages(void *ptr, unsigned int iLen);
//thread系列
#include "api/kgen_thrd.h"
//
static int mythread(char *ptr);
struct task_struct *kgen_task;
int scull_open(struct inode *inode, struct file *filp)
{
	int minor=MINOR(inode->i_rdev);
	//int major=MAJOR(inode->i_rdev);
	printk(KERN_ALERT "*k* scull open\n");
	if(minor==0){
		printk("*k* minor is 0\n");
	}else if(minor==1){
		printk("*k* minor is 1\n");
	}else{
		printk("*k* other minor number,error\n");
		return -1;
	}
	return 0;
} 
char tempbuf[1000]={0};
int scull_release(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "*k* scull close\n");
	return 0;
}
int rw=0;
ssize_t scull_write(struct file *filp,
		const char __user *buf, 
		size_t count, 
		loff_t *f_pos)
{
	copy_from_user(tempbuf,buf,count);
	rw=1;
	//printk(KERN_ALERT "*k* kernel print %s size=%d\n",tempbuf,count);
	return count;
}
int scull_mmap (struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_flags |=VM_RESERVED;
	vma->vm_page_prot=
		pgprot_noncached(vma->vm_page_prot);
	printk("*k* vma->vm_start 0x%lx\n",vma->vm_start);
	printk("*k* vma->vm_end 0x%lx\n",vma->vm_end);
	printk("*k* vma->vm_pgoff 0x%lx\n",vma->vm_pgoff);
	if(remap_pfn_range(vma,
				vma->vm_start,
				vma->vm_pgoff,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot)){
		printk("*k* remap page range failed\n");
		return -ENXIO;
	}
	return 0;
}

ssize_t scull_read(struct file *filp,
		char __user *buf, 
		size_t count, 
		loff_t *f_pos)
{
	if(rw==0){
		return -1;
	}
	copy_to_user(buf,tempbuf,count);
	rw=0;
	return count;
}
//简单ioctl 置errno
//#define SIMPL_IOCTL
#ifdef SIMPL_IOCTL
int scull_ioctl(struct inode *inode,struct file *filep,
		unsigned int cmd,unsigned long arg)
{
	printk(".....cmd is 0x%X,arg is 0x%X arg=%s\n",cmd,arg,arg);

	return -23;

	struct st{
		int len;
		char *p;
	};
#define MAX_ARG_LEN 10
	struct st st1;
	char buff[MAX_ARG_LEN];
	int cpret=0;
	//从用户空间复制结构体到内核空间
	copy_from_user(&st1,(struct st *)arg,sizeof(struct st));
	if(st1.len>MAX_ARG_LEN){
		printk("arg too long,it must less than %d\n",MAX_ARG_LEN);
		return -1001;
	}
	//从结构体分解出长度和值;复制到本地缓冲区
	cpret=copy_from_user(buff,st1.p,st1.len);
	printk("cpret=%d\n",cpret);
	switch(cmd){
		case 1:
			printk("*k* cmd 1,arg %s\n",buff);
			break;
		case 2:
			printk("*k* cmd 2,arg %s\n",buff);
			break;
		case 3:
			printk("*k* cmd 3,arg %s\n",buff);
			break;
		default:
			printk(KERN_ALERT "*k* unknow cmd:%d\n",cmd);
			return -99;
	}
	return 99;
}
#endif

//内核重映射+非占用内核
#define Other_ioctl
#ifdef Other_ioctl
int scull_ioctl(struct inode *inode,struct file *filep,
		unsigned int cmd,unsigned long arg)
{
	void *paddr=NULL;//重映射地址
	int i;
	int ledno=0;
	int getled;
	struct task_struct *kgen_task;
	printk("cmd=%d arg=%ld\n",cmd,arg);
	//第一个参数 第几个LED
	switch(cmd){
		case 1:
			ledno=0x10;//0b0001 0000
			break;
		case 2:
			ledno=0x20;//0b0010 0000
			break;
		case 3:
			ledno=0x40;//0b0100 0000
			break;
		case 4:
			ledno=0x80;//0b1000 0000
			break;
		case 5:	//利用ioctl创建线程
			kgen_task =
				kthread_run((void *)mythread,
						"kthread arg2","thread1");
			return 0;
			break;
		default:
			printk("err led number,must be 1~4\n");
			return -1001;
	}
	//地址重映射到内核态
	//ioremap();
	paddr=ioremap_nocache(0x56000054/*LED*/,4096);
	*(int *)paddr=0xff;
	printk("the led will flash %ld time(s)\n",arg);
	//将第二个参数设置成为循环次数
	for(i=0;i<arg*2;i++){
		printk("count=%d \n",i);
		//非占用的延时
		set_current_state(TASK_INTERRUPTIBLE);
		//等到HZ个调度周期=1秒
		schedule_timeout(HZ/4); //HZ是宏定义
		//取状态
		getled=((*(int *)paddr )& ledno)>>(cmd-1+4) ; 
		//反转状体
		if(getled){//1 then clear	
			*(int *)paddr &= (~(1 << (cmd-1+4)));
		}else{//0 then set 1
			*(int *)paddr |= (1 << (cmd-1+4));
		}

	}
	//释放重映射表
	iounmap(paddr);
	return 0;
}
#endif
//一系列函数指针,指向驱动操作的种种
struct file_operations fops=
{
	.owner=THIS_MODULE,
	.open=scull_open,
	.release =scull_release,
	.write=scull_write,
	.read=scull_read,
	.mmap=scull_mmap,
	.ioctl=scull_ioctl
};
//中断服务程序
irqreturn_t drv_int_handler(int irq,void * dev_id)
{
	printk("drv int0\n");
	return IRQ_HANDLED;
}
//中断服务程序2
irqreturn_t drv_int_handler_eint1(int irq,void * dev_id)
{
	printk("drv int1 \n");
	//kgen_thread_free(kgen_task);
	printk("after stop thread  \n");
	return IRQ_HANDLED;
}
static int mythread(char *ptr)
{
	int i;
	printk("ptr is %s\n",ptr);
	for(i=0;i<10;i++){
		printk("kthread\n");
		//非占用的延时
		set_current_state(TASK_INTERRUPTIBLE);
		//等到HZ个调度周期=1秒
		schedule_timeout(HZ); //HZ是宏定义
		//情况2 检查停止信号
		if(kthread_should_stop())//检查是否需要(提前)退出
			break;
	}
	//如果线程函数永远不返回并且不检查信号，它将永远都不会停止。
	return 0; // 情况1 返回
}
static int mythread_api(void *ptr)
{
	int i;
	printk("ptr is \"%s\"\n",(char *)ptr);
	for(i=0;i<10;i++){
		printk("kthread\n");
		//非占用的延时
		set_current_state(TASK_INTERRUPTIBLE);
		//等到HZ个调度周期=1秒
		schedule_timeout(HZ); //HZ是宏定义
		//情况2 检查停止信号
		if(kthread_should_stop())//检查是否需要(提前)退出
			break;
	}
	//如果线程函数永远不返回并且不检查信号，它将永远都不会停止。
	return 0; // 情况1 返回
}
// __init调用后释放内存
static int __init hello_init(void)
{
	
	char* ptr=NULL;
	int len=0x100000;//1M空间
	struct semaphore g_sem;
	int ret;
	int i;
	int ulVAddr=0;
	// ************************<IRQ> ********************************
	//int rv=0;
	// 
	ulVAddr=(int)ioremap_nocache(0x56000050,4096);//GPFCON
	*(int*)ulVAddr=0x55aa;//设置成中断模式[输入/输出/中断]
	//释放映射
	iounmap((void *)ulVAddr);
	//
	ulVAddr=(int)ioremap_nocache(0x56000088,4096);//EXTINT0 
	*(int*)ulVAddr &= 0xfffffe00;//设置中断类别[高低/跳变]
	*(int*)ulVAddr |= 0x1b6;//0b1 1011 0110= 110 110 110 
	iounmap((void *)ulVAddr);
	//先注销
	free_irq(IRQ_EINT0,(void *)0);
	//request_irq 注册中断服务
	/* 1 发生这个中断时
	 * 2 调用这个中断服务程序
	 * 3 指定快速中断标志flags
	 * 4 通常是设备驱动程序的名称
	 * 5 可作为共享中断时的中断区别参数，
	 *   也可以用来指定中断服务函数需要参考的数据地址
	 * */
	if(request_irq(IRQ_EINT0,drv_int_handler,SA_INTERRUPT,"int1",NULL)){
		printk("INT0 enable error \n");
	}
	//中断服务表源于 源代码驱动中 utulinux/drive/input/keybord中定义 	
	free_irq(IRQ_EINT1,(void *)1);
	if(request_irq(IRQ_EINT1,
				drv_int_handler_eint1,
				SA_INTERRUPT,
				"int1",(void *)1)){
		printk("INT1 enable error \n");
	}	
	//</IRQ>
	//****************************** 内存分配 内核*****************
	// 1 简单kmalloc
	ptr=(char *)kmalloc(1000,GFP_KERNEL);
	printk("ptr=%p\n",ptr);
	memset(ptr,0,1000);
	*ptr='a';*(ptr+1)='b';*(ptr+2)='\0';
	printk("ptr(str)=%s\n",ptr);
	kfree(ptr);
	// 2 DMA no cache 为不设备可以直接访问
	ptr=(char *)kmalloc(1000,GFP_KERNEL|GFP_DMA);
	printk("ptr=%p\n",ptr);
	kfree(ptr);
	// 3 大于 128k后备(分配大内存)
	//基本业 4096 参数2= 2^<参数2>页 例如2^0*4096[byte]
	//分配1M计算麻烦 系统提供get_order
	printk("get order %d\n",get_order(len));
	ptr=(char *)__get_free_pages(GFP_KERNEL,get_order(len));//分配
	memset(ptr,'b',len);//使用
	*(ptr+len-1)='\0';
	//printk("ptr(str)=%s\n",ptr);	
	printk("sizeof big malloc(str) 0x%x\n",strlen(ptr));
	free_pages((int)ptr,get_order(len));//释放
//#define THREAD
#ifdef THREAD
	//****************** 创建内核线程*********************************
	//创建并唤醒 create and warkup
	//参数3 线程名字 ps看到的
	kgen_task =kthread_run((void *)mythread,"kthread arg","thread1");
	//非占用的延时
	set_current_state(TASK_INTERRUPTIBLE);
	//等到HZ个调度周期=1秒
	schedule_timeout(HZ); //HZ是宏定义
	ret=kthread_stop(kgen_task); //等待线程结束
	printk("stop thread ret %d\n",ret);
#endif
#define MUTEX
#ifdef MUTEX
	//*************************互斥锁 **************************
	init_MUTEX(&g_sem);//初始化互斥锁
	ret=down_interruptible(&g_sem);//锁
	printk("down interrupt return %d\n",ret);
	ret=down_trylock(&g_sem);
	printk("down trylock return %d\n",ret);
	up(&g_sem);//解锁
#endif
	//************************* 初始化模块时调用******************
	printk(KERN_ALERT "*k* insmod:mem_test.ko\n");
	//注册
	ret=register_chrdev(422, "my char dev ", &fops);
	if(ret<0){
		printk("error\n");
		return -1;
	}
	//********************平台 api调用 才测试****************
	printk("************* plat test *****************\n");
	test_func_api();
#define KGEN_MALLOC_LEN 128*1024
	ptr=kgen_malloc(KGEN_MALLOC_LEN);
	if(ptr!=NULL){
		memset(ptr,'a',KGEN_MALLOC_LEN);
		printk("kgen_malloc OK char=%c \n",*(ptr+10));
		kgen_free(ptr);
	}else{
		printk("kgen_malloc ERR \n");
	}
#define KGEN_MALLOC_PAGES_LEN 128*1024
	ptr=kgen_malloc_pages(KGEN_MALLOC_PAGES_LEN);
	if(ptr!=NULL){
		memset(ptr,'c',KGEN_MALLOC_PAGES_LEN);
		printk("kgen_malloc_pages OK char=%c \n",*(ptr+1000));
		kgen_free_pages(ptr,KGEN_MALLOC_PAGES_LEN);
	}else{
		printk("kgen_malloc_pages ERR \n"); 
	}
	//*******  线程测试
	printk("**** gen_thrd ***** \n"); 
	kgen_task=kgen_thread_new(
		"threadname",0,0,
		&mythread_api,
		(void *)"i am arg :)");
	//非占用的延时
	set_current_state(TASK_INTERRUPTIBLE);
	//等到HZ个调度周期=1秒
	schedule_timeout(HZ*2); //HZ是宏定义
	kgen_thread_free(kgen_task);
	//kgen_thread_free(kgen_task);
	return 0;
}
static void hello_exit(void)
{
	printk(KERN_ALERT "*k* rmmod: mem_test.ko\n");
	//反注册
	unregister_chrdev(422, "my char dev");

}
//模块初始化
module_init(hello_init);
//模块退出函数
module_exit(hello_exit);

