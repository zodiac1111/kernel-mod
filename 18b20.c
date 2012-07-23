/* 	file:18b20.c
 *  	简单调用自制驱动模块,
 *  	应用层调用.
 *  	调用之前 需要首先加载好模块 并创建好设备节点,
 *  	否则打开会失败.(其他情况当然也可能导致打开失败 :) )
 *  	2
 *  	read write通过全局变量传递长度.一个线程写,另一个线程读
 *  	3
 *  	mmap物理内存映射到应用层
 * */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <ctype.h>
//#define GPJCON 0x560000d0 
//设置寄存器某一位为1.
//REG 寄存器 BIT 设置的某一位为1
#define SET_BIT(REG,BIT) \
	(REG) |= 0x1<(BIT)
#define CLR_BIT(REG,BIT) \
	(REG) &= ~(0x1<(BIT))

unsigned int reset_ds18b20(void);
struct sarg{
	int fd;
	int len;
};
int glen=0;
int volatile addrGPJCON =0;//
int volatile addrGPJDAT =0;//
/*配置为输入模式*/
void set_conIN(void)
{// 00
	*(int *)addrGPJCON &= ~(1<<0);
	*(int *)addrGPJCON &= ~(1<<1);
}
/*配置为输出模式*/
void set_conOUT(void)
{// 01
	*(int *)addrGPJCON |= (1<<0);
	*(int *)addrGPJCON &= ~(1<<1);
}
/*引脚置位*/
void set_data(int i)
{
	if( i == 0 ){
		*(int *)addrGPJDAT &= ~(1<<0);
	}else if( i == 1 ){
		*(int *)addrGPJDAT |= (1<<0);
	}
}
int main()
{
	int fd=0;
	int i=0;
	fd=open("/dev/my",O_RDWR);
	printf("fd is %d\n",fd);
	if(fd==-1){
		printf("mod [mem_test] is load? or did you mknod\n");

	}
	int *addr=NULL;
	int idat=0;
	//linux 要求页对齐,so 5600000000
	addr=mmap((void*)0,4096,PROT_WRITE,MAP_SHARED,fd,0x56000000);
	printf("addr 0x%p\n",addr);
	//TODO 更改控制寄存器
	addrGPJCON =(int)addr+0xd0;//
	addrGPJDAT =(int)addr+0xd4;//
	idat=reset_ds18b20();
	return 0;
	//***** test for gpj0 output ***********
	//GPJ0 -(out)-> GPJ1
	set_conOUT();//GPJ0 out->
	while(1){
		i++;
		if(i%2==0){
			set_data(0);
			printf("set j0=0 ");
		}else{
			set_data(1);
			printf("set j0=1 ");
		}
		// GPJ1 <- in(reset value)
		//idat=*(int *)(addrGPJDAT);
		printf(" data is 0x%08x \n",*(int *)addrGPJDAT);
		usleep(1000000);
	}
	close(fd);
	return 0;
}
unsigned int reset_ds18b20(void)
{
	unsigned int retValue;
	set_conOUT();
	set_data(1);
	usleep(1);
	printf("1 out 1 0x%08x\n",*(int *)addrGPJDAT);
	set_data(0);
	usleep(600);// 480us minimum
	printf("2 out 0+600 0x%08x\n",*(int *)addrGPJDAT);
	set_data(1);
	printf("2.5 out 1 0x%08x\n",*(int *)addrGPJDAT);
	//usleep(20);
	set_conIN();
	printf("3 in 0x%08x\n",*(int *)addrGPJDAT);
	usleep(60);
	printf("4 in+60 0x%08x\n",*(int *)addrGPJDAT);
	usleep(100);
	printf("5 in+100 0x%08x\n",*(int *)addrGPJDAT);
	//usleep(10);
	/*稍做延时后 如果
	 * x=0则初始化成功
	 * x=1则初始化失*/
	retValue = ((*(int *)addrGPJDAT ) & 0x01);
	printf("18b20 init is 0x%08x\n",*(int *)addrGPJDAT);
	printf("18b20 init is %d\n",retValue);
	return retValue;
}


