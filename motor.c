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
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/kthread.h>
//#include <linuxev.h>
#include <asm/arch/regs-gpio.h>
#include <asm-arm/arch/regs-gpioj.h>
#include <asm/arch/regs-irq.h>
#include <asm/uaccess.h>	/* copy_to_user() */
#include <linux/delay.h>	// mdelay()
#include <linux/irq.h>
#include <linux/device.h>	//include class_create device_create
#define IOCMD_MOTOR_DIR_FORWARD 0	//电机正转
#define IOCMD_MOTOR_DIR_REVERSE 1	//电机反转
//调试开关
//#define DEBUG
#define	GPJCON (*(volatile unsigned int *)S3C2440_GPJCON)
#define	GPJDAT (*(volatile unsigned int *)S3C2440_GPJDAT)
#define	GPJUP  (*(volatile unsigned int *)S3C2440_GPJUP)
//步进电机4相位
#define P_A 0x1
#define P_B 0x2
#define P_C 0x4
#define P_D 0x8

#define DEVICE_NAME "motor"
static int major = 0, minor = 0;	//主、次设备号
static struct class *dev_class;

/*配置为输出模式*/
void set_conAllOUT(void)
{				
	GPJCON &= ~0xFF;	//低8位置零
	GPJCON |= 0x55;		//低8位设置0101 0101 // 0123口[输出]
}

//all引脚置位
void set_alldata(int i)
{
	GPJDAT &= ~0xFF;
	GPJDAT |= i;
}

void mymdelay(int i)
{
	//非占用的延时
	set_current_state(TASK_INTERRUPTIBLE);
	//等到HZ个调度周期=1秒 HZ这个内核设置是200
	//太小没意义
	schedule_timeout(i * HZ / 1000);	//HZ是宏定义
	return;
}

//使用IO(下策) 控制发出的脉冲 p:相位 delay:延时
//TODO PWM
void pulse(unsigned char p, unsigned char delay)
{
	set_alldata(p);
	//非占用的延时
	set_current_state(TASK_INTERRUPTIBLE);
	//等到HZ个调度周期=1秒 HZ这个内核设置是200
#define MOTOR_DELAY 1
	schedule_timeout(MOTOR_DELAY * HZ / 2000);	//HZ是宏定义
	return;
}

//向某个方向旋转1个step
void step(unsigned char dir, unsigned char delay)
{
	if (dir == 0) {
		pulse(P_A | P_B, delay);
		pulse(P_B | P_C, delay);
		pulse(P_C | P_D, delay);
		pulse(P_D | P_A, delay);
	} else {
		pulse(P_D | P_A, delay);
		pulse(P_C | P_D, delay);
		pulse(P_B | P_C, delay);
		pulse(P_A | P_B, delay);
	}
	return;
}

int scull_open(struct inode *inode, struct file *filp)
{
	
#ifdef DEBUG
	printk(KERN_ALERT "*k* open motor\n");
#endif
	return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	printk(KERN_ALERT "*k* close motor\n");
#endif
	return 0;
}

ssize_t
scull_write(struct file * filp,
	    const char __user * buf, size_t count, loff_t * f_pos)
{
	//printk(KERN_ALERT "*k* kernel print %s size=%d\n",tempbuf,count);
	return 0;
}

ssize_t
scull_read(struct file * filp, char __user * buf, size_t count, loff_t * f_pos)
{
	return 0;
}

//
int
scull_ioctl(struct inode *inode, struct file *filep,
	    unsigned int cmd, unsigned long arg)
{

	int i;
	int stepMotorspeed = 2;
#ifdef DEBUG
	printk("*k* cmd=%d arg=%ld\n", cmd, arg);
#endif
	//第一个参数 第几个LED
	switch (cmd) {
		//步进电机
	case IOCMD_MOTOR_DIR_FORWARD:	//:正转
		if (arg < 3) {
			printk("WARNING the dergee is too small,set arg as 3\n");
			//return 0;
			arg = 3;
		}
		set_conAllOUT();
		for (i = 0; i < arg; i++) {
			step(0, stepMotorspeed);
		}
		set_alldata(0);
		break;
	case IOCMD_MOTOR_DIR_REVERSE:	//:反转
		if (arg < 3) {
			printk("WARNING:the dergee is too small,set arg as 3\n");
			//return 0;
			arg = 3;
		}
		set_conAllOUT();
		for (i = 0; i < arg; i++) {
			step(1, stepMotorspeed);
		}
		set_alldata(0);
		break;
	default:
		printk("*k* err : unknow command \n");
		return -1001;
	}
	return 0;
}
//一系列函数指针,指向驱动操作的种种
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.release = scull_release,
	.write = scull_write,
	.read = scull_read,
	.ioctl = scull_ioctl
};

// __init调用后释放内存
static int __init hello_init(void)
{
	//动态注册
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk("register %s error\n", DEVICE_NAME);
		return 1;
	}
	printk("Dev number :%d, %d\n", major, minor);
	//创建设备节点
	dev_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (NULL == dev_class) {
		printk("device class create fail\n"
		       "TODO::mknod /dev/%s c %d %d\n",
		       DEVICE_NAME, major, minor);
	} else {
		device_create(dev_class, NULL, MKDEV(major, minor), "%s%d",
			      DEVICE_NAME, minor);
		printk("/dev/%s%d register success\n", DEVICE_NAME, minor);
	}
	return 0;
}

static void hello_exit(void)
{
	if (dev_class) {
		device_destroy(dev_class, MKDEV(major, minor));
		class_destroy(dev_class);
	}
	unregister_chrdev(major, DEVICE_NAME);
}

//模块初始化
module_init(hello_init);
//模块退出函数
module_exit(hello_exit);
