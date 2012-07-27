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
#define IOCMD_MOTOR_DIR_FORWARD 4	//电机正转
#define IOCMD_MOTOR_DIR_REVERSE 5	//电机反转
#define TEMP_PIN 6		//温度端口(GPJ口)
#define DEVICE_NAME "my"
static int major = 0, minor = 0;	//主、次设备号
static struct class *dev_class;
static char tempbuf[1000] = { 0 };
static int rw = 0;
/*配置为输入模式*/
void set_conIN(void)
{				//00
	GPJCON &= ~(0x1 << TEMP_PIN*2);
	GPJCON &= ~(0x1 << (TEMP_PIN*2 + 1));
}

/*配置为输出模式*/
void set_conOUT(void)
{				//01
	GPJCON |= (0x1 << TEMP_PIN*2);
	GPJCON &= ~(0x1 << (TEMP_PIN*2 + 1));
}

/*配置为输出模式*/
void set_conAllOUT(void)
{				
	GPJCON &= ~0xFF;	//低8位置零
	GPJCON |= 0x55;		//低8位设置0101 0101 // 0123口[输出]
	//GPJCON &= ~(1<<1);
}

/*引脚置位*/
void set_data(int i)
{
	if (i == 0) {
		GPJDAT &= ~(1 << TEMP_PIN);
	} else if (i == 1) {
		GPJDAT |= (1 << TEMP_PIN);
	}
}

//all引脚置位
void set_alldata(int i)
{
	GPJDAT &= ~0xFF;
	GPJDAT |= i;
}

//设置引脚电平
void setGPJ6(int b)
{
	if (b != 0) {
		GPJDAT |= (0x1 << 6);
	} else {
		GPJDAT &= ~(0x1 << 6);
	}
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
	if (dir == 1) {
		pulse(P_D | P_A, delay);
		pulse(P_C | P_D, delay);
		pulse(P_B | P_C, delay);
		pulse(P_A | P_B, delay);
	} else {
		pulse(P_A | P_B, delay);
		pulse(P_B | P_C, delay);
		pulse(P_C | P_D, delay);
		pulse(P_D | P_A, delay);
	}
	return;
}

int scull_open(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);
	//int major=MAJOR(inode->i_rdev);
	printk(KERN_ALERT "*k* scull open\n");
/*	
	if (minor == 0) {
		printk("*k* minor is 0\n");
	} else if (minor == 1) {
		printk("*k* minor is 1\n");
	} else {
		printk("*k* other minor number,error\n");
		return -1;
	}
*/
	return 0;
}

unsigned int reset_ds18b20(void)
{
	unsigned int retValue;
	set_conOUT();
	set_data(1);
	udelay(1);
#ifdef DEBUG
	printk("1 out 1 0x%08x\n", GPJDAT);
#endif
	set_data(0);
	udelay(500);		// 480us minimum
#ifdef DEBUG
	printk("2 out 0+480 0x%08x\n", GPJDAT);
#endif
	//set_data(1);
	//printk("2.5 out 1 0x%08x\n",GPJDAT);
	//udelay(60);
	set_conIN();
	//printk("3 in 0x%08x\n",GPJDAT);
	udelay(60);
	/*稍做延时后 如果
	 * x=0则初始化成功
	 * x=1则初始化失*/
	//注意!小心死循环! 不可取      
	//while((GPJDAT ) & 0x01){
	//      ;
	//}
	//取得TEMP_PIN位的状态
	retValue = ((GPJDAT) & (0x01<<TEMP_PIN))>>TEMP_PIN;
#ifdef DEBUG
	printk("18b20 init is 0x%08x\n", GPJDAT);
#endif
	udelay(500);		//后延
	return retValue;
}

//读取1位
unsigned int read_bit(void)
{
	int ret = 0;
	set_conOUT();
	set_data(0);		//拉低
	udelay(2);		//大于1us
	//set_data(1);//? 拉高?为什么
	set_conIN();		//释放
	udelay(8);
	ret = ((GPJDAT) & (0x01 <<TEMP_PIN));
	udelay(80);
	udelay(2);
	return ret;
}

int scull_release(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "*k* scull close\n");
	return 0;
}

ssize_t
scull_write(struct file * filp,
	    const char __user * buf, size_t count, loff_t * f_pos)
{
	copy_from_user(tempbuf, buf, count);
	rw = 1;
	//printk(KERN_ALERT "*k* kernel print %s size=%d\n",tempbuf,count);
	return count;
}

ssize_t
scull_read(struct file * filp, char __user * buf, size_t count, loff_t * f_pos)
{
	if (rw == 0) {
		return -1;
	}
	copy_to_user(buf, tempbuf, count);
	rw = 0;
	return count;
}

/*写一位命令*/
void write_bit(char bitValue)
{
	//TODO 弄清楚怎么写
	// see p13 write time slots
	set_conOUT();
	set_data(0);
	udelay(5);
	if (bitValue == 1) {
		//udelay(2);//>1us <15us
		set_data(1);
	} else {
		set_data(0);
	}
	udelay(61);		//>60//保持拉低60
	set_data(1);
	set_conIN();		//释放
	udelay(2);		//slot间间隔
	return;
}

// 写命令 1字节
void write_cmd(unsigned char cmd)
{
	unsigned char i;
	unsigned char temp;
	for (i = 0; i < 8; i++) {
		temp = cmd >> i;	//从低位到高位写
		temp &= 0x01;
		write_bit(temp);
	}
	return;
}

//串行读取一个字节
char read_char(void)
{
	int i;
	char value = 0;
	for (i = 0; i < 8; i++) {
		if (read_bit()) {
			//也是从低地址位到高地址位
			value |= (0x01 << i);
		}
	}
	return value;
}

//内核重映射+非占用内核
int
scull_ioctl(struct inode *inode, struct file *filep,
	    unsigned int cmd, unsigned long arg)
{

	int i;
	short value;		//short=2 Byte
	char lowValue = 0, highValue = 0;
	//int ret = 0;
	int stepMotorspeed = 2;
	printk("*k* cmd=%d arg=%ld\n", cmd, arg);
	//第一个参数 第几个LED
	switch (cmd) {
	case 1:		//复位并开始转换
		break;
	case 2:
		break;
	case 3:		//温度传感器
		if (reset_ds18b20()) {
			printk("init 18b20 error\n");
			return -2;
		}
		set_conOUT();
		set_data(1);
		write_cmd(0xCC);
		write_cmd(0x44);
		mymdelay(1000);	//等待
		if (reset_ds18b20()) {
			printk("init 18b20 error\n");
			return -3;
		}
		set_conOUT();
		set_data(1);
		//mdelay(400);
		write_cmd(0xCC);
		write_cmd(0xBE);
		// 读取温度转化数值
		lowValue = read_char();
		highValue = read_char();
#ifdef DEBUG
		printk("lowValue is %d[0x%x]\n", lowValue, lowValue);
		printk("highValue is %d[0x%x]\n", highValue, highValue);
#endif
		value = highValue * 256 + lowValue;
		//highValue |= ((lowValue&0xf0)>>4);
		printk("kernel is %d\n", value);
		return value;
		break;
		//步进电机
	case IOCMD_MOTOR_DIR_FORWARD:	//4:正转
		if (arg < 3) {
			printk("the dergee is too small\n");
			//return 0;
			arg = 3;
		}
		set_conAllOUT();
		for (i = 0; i < arg; i++) {
			step(1, stepMotorspeed);
		}
		set_alldata(0);
		break;
	case IOCMD_MOTOR_DIR_REVERSE:	//5:反转
		set_conAllOUT();
		for (i = 0; i < arg; i++) {
			step(0, stepMotorspeed);
		}
		set_alldata(0);
		break;
		//电磁铁
	case 6:		//
		GPJCON &= ~(0x3 << 12);	//置零 11 oooo oooo oooo
		GPJCON |= 0x1 << 12;	//置01oo oooo oooo// 6pin out
		setGPJ6(arg);	//zero or one Digit out.
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

	// ************************* 初始化模块时调用******************
	int ret;
	printk(KERN_ALERT "*k* insmod:mem_test.ko\n");
	//注册
	ret = register_chrdev(422, "my char dev ", &fops);
	if (ret < 0) {
		printk("error\n");
		return -1;
	}
	return 0;

 /*	//备用的注册方式,注册+创建节点.这个方法更好,但目前没有能力
	//暂时不需要健壮.留作参考.
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk("register %s error\n", DEVICE_NAME);
		return 1;
	}
	printk("Dev number :%d, %d\n", major, minor);

	//创建设备节点
	dev_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (NULL == dev_class) {
		printk
		    ("device class create fail\nTODO::mknod /dev/%s c %d %d\n",
		     DEVICE_NAME, major, minor);
	} else {
		device_create(dev_class, NULL, MKDEV(major, minor), "%s%d",
			      DEVICE_NAME, minor);
		printk("/dev/%s%d register success\n", DEVICE_NAME, minor);
	}

	return 0;
*/
}

static void hello_exit(void)
{
	//释放重映射表
	//iounmap(paddr);
	printk(KERN_ALERT "*k* rmmod: mem_test.ko\n");
	//反注册
	unregister_chrdev(422, "my char dev");

}

//模块初始化
module_init(hello_init);
//模块退出函数
module_exit(hello_exit);
