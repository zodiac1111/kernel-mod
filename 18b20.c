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


int main()
{
	int fd=0;
	int i=0;
	int ret;
	char cmd=0;
	char buf=0;
	printf("start\n");
	fd=open("/dev/DS18B20-drv0",O_RDWR);
	printf("fd=%d\n",fd);
	ret =ioctl(fd,0,0);
	printf("ret=%d\n",ret);
	cmd=0xCC;
	write(fd,&cmd,1);
	cmd=0x44;
	write(fd,&cmd,1);
	sleep(1);
	ret =ioctl(fd,0,0);
	cmd=0xCC;
	write(fd,&cmd,1);
	cmd=0xBE;
	write(fd,&cmd,1);
	read(fd,&buf,1);
	printf("value1=%x\n",buf);
	read(fd,&buf,1);
	printf("value2=%x\n",buf);
	close(fd);
	return 0;
}



