/* filename setreg.c
 * kernel module program source code
 * 内核模块直接读写物理寄存器
 * 通过ioremap_nocache实现
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
#define DEBUG
#define DEVICE_NAME "setreg"	//默认设备名称 ts0 ts1 ts2...
static int major = 0;
static int minor = 0;		//主、次设备号
static struct class *dev_class;
static int scull_open(struct inode *inode, struct file *filp);
static int scull_release(struct inode *inode, struct file *filp);
static ssize_t
scull_read(struct file * filp, char __user * buf, size_t count, loff_t * f_pos);
static ssize_t
scull_write(struct file * filp,
	    const char __user * buf, size_t count, loff_t * f_pos);
static int
scull_ioctl(struct inode *inode, struct file *filep,
	    unsigned int cmd, unsigned long arg);
int scull_mmap (struct file *filp, struct vm_area_struct *vma);


//一系列函数指针,指向驱动操作的种种
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.release = scull_release,
	.write = scull_write,
	.read = scull_read,
	.ioctl = scull_ioctl,
	.mmap=scull_mmap
};
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
static int
scull_ioctl(struct inode *inode, struct file *filep,
	    unsigned int addr, unsigned long val)
{


	return 0;
}

static int scull_open(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	printk(KERN_ALERT "*k* temperature sensor open \n");
#endif
	return 0;
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
