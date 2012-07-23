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
#include <asm-arm/arch/regs-gpioj.h>
#include <asm/arch/regs-irq.h>
#include <asm/uaccess.h> /* copy_to_user() */
#include <linux/delay.h>	/* mdelay() */
#include <linux/irq.h>

#define	GPJCON (*(volatile unsigned int *)S3C2440_GPJCON)
#define	GPJDAT (*(volatile unsigned int *)S3C2440_GPJDAT)
#define	GPJUP (*(volatile unsigned int *)S3C2440_GPJUP)
/*配置为输入模式*/
void set_conIN(void)
{// 00
	GPJCON &= ~(1<<0);
	GPJCON &= ~(1<<1);
}
/*配置为输出模式*/
void set_conOUT(void)
{// 01
	GPJCON |= (1<<0);
	GPJCON &= ~(1<<1);
}
/*引脚置位*/
void set_data(int i)
{
	if( i == 0 ){
		GPJDAT &= ~(1<<0);
	}else if( i == 1 ){
		GPJDAT |= (1<<0);
	}
}
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
unsigned int reset_ds18b20(void)
{
	unsigned int retValue;
	set_conOUT();
	set_data(1);
	udelay(1);
	printk("1 out 1 0x%08x\n",GPJDAT);
	set_data(0);
	udelay(480);// 480us minimum
	printk("2 out 0+480 0x%08x\n",GPJDAT);
	//set_data(1);
	//printk("2.5 out 1 0x%08x\n",GPJDAT);
	//udelay(20);
	set_conIN();
	//printk("3 in 0x%08x\n",GPJDAT);
	udelay(60);

	//printk("4 in+60 0x%08x\n",*(int *)GPJDAT);
	//udelay(1);
	//printk("5 in+1 0x%08x\n",*(int *)GPJDAT);
	//usleep(10);
	/*稍做延时后 如果
	 * x=0则初始化成功
	 * x=1则初始化失*/
	retValue = -((GPJDAT ) & 0x01);
	printk("18b20 init is 0x%08x\n",GPJDAT);
	//udelay(1);
	printk("18b20 init is 0x%08x\n",GPJDAT);
	printk("18b20 init is 0x%08x\n",GPJDAT);
	printk("18b20 init is 0x%08x\n",GPJDAT);
	printk("18b20 init is %d\n",retValue);
	udelay(480);
	return retValue;
}
unsigned int read_bit(void)
{
	int ret=0;
	set_conOUT();
	//set_data(1);
	//__udelay(2);
	set_data(0);//拉低
	udelay(2); //大于1us
	set_conIN(); //释放
	udelay(8);
	ret=((GPJDAT) & 0x01);
	udelay(50);
	udelay(2);
	return ret;
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
/*写一位命令*/
void write_bit(char bitValue)
{
// see p13 write time slots
	set_conOUT();
	//set_data(1);
	//udelay(1);
	set_data(0);
	//udelay(15);
	if( bitValue == 1 ){
		udelay(2);//>1us <15us
		set_conIN();//释放
		udelay(61);
	}else{
		//set_data(0);
		udelay(63);//>60//保持拉低60
		set_conIN();//释放
	}
	udelay(2);//slot间间隔
}
/*写命令 1字节*/
void write_cmd(unsigned char cmd)
{
	unsigned char i;
	unsigned char temp;
	for(i=0; i<8;i++){
		temp = cmd>>i;
		temp &= 0x01;
		write_bit(temp);
	}
	//__udelay(10);
}

//内核重映射+非占用内核
#define Other_ioctl
#ifdef Other_ioctl
int scull_ioctl(struct inode *inode,struct file *filep,
		unsigned int cmd,unsigned long arg)
{

	int i;int value;
	char lowValue=0,highValue=0;
	int ledno=0;
	int ret=0;
	printk("cmd=%d arg=%ld\n",cmd,arg);
	//第一个参数 第几个LED
	switch(cmd){
		case 1:
			reset_ds18b20();
			break;
		case 2:

			for(i=0;i<8;i++){
				ret=read_bit();
				printk("%d ",ret);
				udelay(62);	
			}
			break;
		case 3:
		//	GPJUP=0x0;
		/*	if(reset_ds18b20()){
				printk("init 18b20 error\n");
			}
			udelay(400);
			set_conOUT();
			set_data(1);
			write_cmd(0xCC);
			write_cmd(0x44);
			mdelay(1000);
		*/
			//	while(1){
			if(reset_ds18b20()){
				printk("init 18b20 error\n");
			}
			//udelay(400);
			//set_conOUT();
			//set_data(1);
			write_cmd(0x33);
			//write_cmd(0xCC);
			//write_cmd(0xBE);
			// 读取温度转化数值
			for(i=0; i<8; i++){
				if( read_bit() ){
					lowValue |= (0x01<<i);
				}
				//udelay(62);
			}
			for(i=0; i<8; i++){
				if( read_bit() ){
					highValue |= (0x01<<i);
				}
				//udelay(62);
			}
			printk("lowValue is %d[0x%x]\n",lowValue,lowValue);
			printk("highValue is %d[0x%x]\n",highValue,lowValue);
			highValue <<= 4;
			highValue |= ((lowValue&0xf0)>>4) ;
			printk("kernel is %d\n",highValue);
			
			//mdelay(600);	}
			return highValue;
			break;
			
		case 4:
			ledno=0x80;//0b1000 0000
			break;
		case 5:	//利用ioctl创建线程
			//return 0;
			break;
		default:
			printk("err led number,must be 1~4\n");
			return -1001;
	}



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
	.ioctl=scull_ioctl
};

// __init调用后释放内存
static int __init hello_init(void)
{

	//************************* 初始化模块时调用******************
	int ret;
	printk(KERN_ALERT "*k* insmod:mem_test.ko\n");
	//注册
	ret=register_chrdev(422, "my char dev ", &fops);
	if(ret<0){
		printk("error\n");
		return -1;
	}
	//地址重映射到内核态
	//ioremap();
	//paddr=ioremap_nocache(0x56000000,4096);
	//GPJCON =(int)paddr+0xd0;//
	//GPJCON =(volatile unsigned int *)S3C2440_GPJCON;//
	//GPJDAT =(int)paddr+0xd4;//
	//GPJDAT =(volatile unsigned int *)S3C2440_GPJDAT;//
	//printk("ioremap GPJCON=%x S3C2410_GPJDAT=%x \n",GPJCON,S3C2440_GPJCON);
	// (*(volatile unsigned int *)S3C2410_GPHDAT)=1;
	//printk("kernel ioremap\n");
	return 0;
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

