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
//平台接口头文件
#include "kgen_platform.h"
static int __init  kgen_init(void)
{
	printk(KERN_ALERT "*k* insmod kgen_plat.ko\n");
	return 0;
}
static void kgen_exit(void)
{
	printk(KERN_ALERT "*k* rmmod kgen_plat.ko\n");
	return ;
}

int test_func_api(void)
{
	printk("test api\n");
	return 0;
}
EXPORT_SYMBOL(test_func_api);
//
char *kgen_malloc_pages(unsigned int iLen)
{
	if(iLen <= KGEN_KMALLOC_MAX_SIZE){
		printk("malloc pages is too small\n");
		return NULL;
	}
	return (char *)__get_free_pages(GFP_KERNEL,get_order(iLen));//分配
}EXPORT_SYMBOL(kgen_malloc_pages);

char *kgen_malloc_dma_pages(unsigned int iLen)
{
	if(iLen <= KGEN_KMALLOC_MAX_SIZE){
		printk("malloc pages is too small\n");
		return NULL;
	}
	return 
	(char *)__get_free_pages(GFP_KERNEL|GFP_DMA,get_order(iLen));//分配
}EXPORT_SYMBOL(kgen_malloc_dma_pages);

void kgen_free_pages(void *ptr, unsigned int iLen)
{
	if(ptr==NULL){
		printk("kgen_free_pages err:ptr is NULL\n");
		return ;
	}
	free_pages((int)ptr,get_order(iLen));//释放
	return;
}EXPORT_SYMBOL(kgen_free_pages);

/*分配128k以下字节
 */
char *kgen_malloc(unsigned int iLen)
{
	if(iLen>KGEN_KMALLOC_MAX_SIZE){
		printk("malloc len is bigthen KGEN_KMALLOC_MAX_SIZE \n");
		return NULL;
	}
	return (char *)kmalloc(iLen,GFP_KERNEL);
}EXPORT_SYMBOL(kgen_malloc);
//dma方式分配小内存
char *kgen_malloc_dma(unsigned int iLen)
{
	if(iLen>KGEN_KMALLOC_MAX_SIZE){
		printk("malloc len is bigthen KGEN_KMALLOC_MAX_SIZE:%d \n",
			KGEN_KMALLOC_MAX_SIZE);
		return NULL;
	}
	return (char *)kmalloc(iLen,GFP_KERNEL|GFP_DMA);
	
}EXPORT_SYMBOL(kgen_malloc_dma);
//释放小内存
void kgen_free(void *ptr)
{
	if(ptr !=NULL){
		kfree(ptr);
	}else{
		printk("kgen_free err: arg is NULL\n");
	}
	return ;
}
EXPORT_SYMBOL(kgen_free);

stdKGenTaskId *kgen_thread_new(char *pcThreadName,
			       KGenThreadPriority ePriority, 
                        unsigned int iStackSize,
			int (* fnThreadEntry)(void *data), 
			       void *pArg)
{
	return kthread_run((void *)fnThreadEntry,pArg,pcThreadName);
}EXPORT_SYMBOL(kgen_thread_new);

int kgen_thread_free(stdKGenTaskId *kgen_task)
{
	if(kgen_task==NULL){
		printk("Err on kgen_thread_free:kgen_task is NULL\n");
		return -1;
	}
	return kthread_stop(kgen_task);
}EXPORT_SYMBOL(kgen_thread_free);
//模块初始化
module_init(kgen_init);
//模块退出函数
module_exit(kgen_exit);
