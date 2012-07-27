/* filename ts.c TemperatureSensor
 * temperature sensor module. 
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
#define TEMP_PIN 6		//温度端口(GPJ口)
#define DEVICE_NAME "ts"	//默认设备名称 ts0 ts1 ts2...
static int major = 0;
static int minor = 0;		//主、次设备号
static struct class *dev_class;
// mode input
static void set_conIN(void)
{				//00
	GPJCON &= ~(0x1 << TEMP_PIN * 2);
	GPJCON &= ~(0x1 << (TEMP_PIN * 2 + 1));
}

//mode output
static void set_conOUT(void)
{				//01
	GPJCON |= (0x1 << TEMP_PIN * 2);
	GPJCON &= ~(0x1 << (TEMP_PIN * 2 + 1));
}

//set data
static void set_data(unsigned char i)
{
	if (i == 0) {
		GPJDAT &= ~(1 << TEMP_PIN);
	} else if (i == 1) {
		GPJDAT |= (1 << TEMP_PIN);
	}
}

static void mymdelay(int i)
{
	//非占用的延时
	set_current_state(TASK_INTERRUPTIBLE);
	//等到HZ个调度周期=1秒 HZ这个内核设置是200
	//太小没意义
	schedule_timeout(i * HZ / 1000);	//HZ是宏定义
	return;
}

static int scull_open(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	printk(KERN_ALERT "*k* temperature sensor open \n");
#endif
	return 0;
}

static signed char reset_ds18b20(void)
{
	signed char retValue;
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
	retValue = ((GPJDAT) & (0x01 << TEMP_PIN)) >> TEMP_PIN;
#ifdef DEBUG
	printk("18b20 init is 0x%08x\n", GPJDAT);
#endif
	udelay(500);		//后延
	return retValue;
}

//读取1位
static unsigned char read_bit(void)
{
	unsigned char ret = 0;
	set_conOUT();
	set_data(0);		//拉低
	udelay(2);		//大于1us
	//set_data(1);//? 拉高?为什么
	set_conIN();		//释放
	udelay(8);
	ret = ((GPJDAT) & (0x01 << TEMP_PIN));
	udelay(80);
	udelay(2);
	return ret;
}

static int scull_release(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	printk(KERN_ALERT "*k* release temperature sensor\n");
#endif
	return 0;
}

static ssize_t
scull_write(struct file * filp,
	    const char __user * buf, size_t count, loff_t * f_pos)
{
	return 0;
}

static ssize_t
scull_read(struct file * filp, char __user * buf, size_t count, loff_t * f_pos)
{
	return 0;
}

//写一位
static void write_bit(char bitValue)
{
	//TODO 弄清楚怎么写 done
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
	set_data(1);		//再拉高?
	set_conIN();		//释放
	udelay(2);		//slot间间隔
	return;
}

// 写命令 1 字节
static void write_cmd(unsigned char cmd)
{
	unsigned char i;
	unsigned char temp=0;
	for (i = 0; i < 8; i++) {
		temp = cmd >> i;	//从低位到高位写
		temp &= 0x01;
		write_bit(temp);
	}
	return;
}

//串行读取一个字节
static char read_char(void)
{
	unsigned char i;
	unsigned char value = 0;
	for (i = 0; i < 8; i++) {
		if (read_bit()) {
			//也是从低地址位到高地址位
			value |= (0x01 << i);
		}
	}
	return value;
}

static int
scull_ioctl(struct inode *inode, struct file *filep,
	    unsigned int cmd, unsigned long arg)
{
	unsigned short value;		//short=2 Byte
	unsigned char lowValue = 0; 
	unsigned short highValue = 0;//高位要移位
#ifdef DEBUG
	printk("*k* cmd=%d arg=%ld\n", cmd, arg);
#endif
	//温度传感器
	if (reset_ds18b20()) {
		printk("init1 18b20 error\n");
		return -2;
	}
	set_conOUT();
	set_data(1);
	write_cmd(0xCC);//跳过rom
	write_cmd(0x44);//开始转换
	mymdelay(400);		//转化需要时间.暂时注销
	if (reset_ds18b20()) {
		printk("init2 18b20 error\n");
		return -3;
	}
	set_conOUT();
	set_data(1);
	write_cmd(0xCC);//跳过rom
	write_cmd(0xBE);//读取温度
	// 读取温度转化数值
	lowValue = read_char();
	highValue = read_char();
#ifdef DEBUG
	printk("lowValue is %d[0x%x]\n", lowValue, lowValue);
	printk("highValue is %d[0x%x]\n", highValue, highValue);
#endif
	//value=highValue;
	value = (highValue << 8 )+ lowValue;
#ifdef DEBUG
	printk("kernel is %d\n", value);
#endif
	return value;
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
