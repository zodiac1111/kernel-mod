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
//我的宏定义
#define WTCON 0x53000000
#define SET_EN_WDT(REG) \
	((REG) |= 0x20)
#define CLR_EN_WDT(REG) \
	((REG) &= ~0x20)
#define SET_WDT_RESET(REG) \
	((REG) |= 0x1)
int main()
{
	int iAddr;
	int fd;
	fd=open("/dev/my",O_RDWR);

	if(fd==-1){
		perror("open watch dog timer");
	}

	printf("before mmap\n");
	//内存映射到用户空间,使用户空间可以操作内核空间
	iAddr=(int)mmap((void *)0,4096,PROT_WRITE,MAP_SHARED,fd,WTCON);
	printf("after mmap\n");

	*(volatile int *)(iAddr + 0x04) =0xf000;//0x8000;//WTDAT 计数器值
	printf("after 0x04\n");
	fflush(stdout);

	*(volatile int *)(iAddr + 0x08) =0x8000;//WTCNT 计数器/会慢慢减小
	printf("after 0x08\n");

	//使能WDT
	SET_EN_WDT(*(volatile int *)(iAddr + 0));// 20 01 //WTCON
	//CLR_EN_WDT(*(volatile int *)(iAddr + 0));// 20 01 //WTCON
	
	//使能重启
	*(volatile int *)(iAddr + 0) |=0x1;// 20 01 //WTCON
	
	//分频 0001 1000
	*(volatile int *)(iAddr + 0) |=0x00;// 20 01 //WTCON
	printf("after 0x00 \n");

	printf("before while\n");
	while(1){
		printf("watchdog: %x \n",*(volatile int *)(iAddr + 0x08));
		
		//printf("feed the watchdog\n");
		//*(volatile int *)(iAddr + 0x08) =0x8000;
		
		usleep(100000);
	}
}
