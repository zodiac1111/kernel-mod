/* 	file:app.c
 *  	简单调用自制驱动模块,
 *  	应用层调用.
 *  	调用之前 需要首先加载好模块 并创建好设备节点,
 *  	否则打开会失败.(其他情况当然也可能导致打开失败 :) )
 *  	2
 *  	read write通过全局变量传递长度.一个线程写,另一个线程读
 *  	3
 *  	mmap无力内存映射到应用层
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

void *funw(void *st);
void *funr(void *st);
struct sarg{
	int fd;
	int len;
};
int glen=0;
int main()
{
	int fd=0;
	int iret=0;
	unsigned int ulVAddr=0;
	char buff[256];
	struct sarg st;
	char seed;
	srand(seed);
	pthread_t ptdw;
	pthread_t ptdr;
	fd=open("/dev/my",O_RDWR);
	printf("fd is %d\n",fd);
	if(fd==-1){
		printf("mod is load? or did you mknod\n");

	}
	//printf("start to write \n");

	//	iret=write(fd,"app write to kernel",strlen("app write to kernel")+1);
	//	printf("write ret is %d.\n",iret);
	//	usleep(1000);
#ifdef TWO
	st.fd=fd;
	st.len=10;
	//	pthread_create(&ptdw,NULL,funw,(void *)&st);
	pthread_create(&ptdr,NULL,funr,(void *)&st);
	while(1){
		funw((void *)&st);
	}
#endif
	int *addr=NULL;
	int *addrbtn=NULL;
	//linux 要求页对齐,so 5600000000
	addr=mmap((void*)0,4096,PROT_WRITE,MAP_SHARED,fd,0x56000000);
	printf("addr 0x%p\n",addr);
	addrbtn=mmap((void*)0,4096,PROT_WRITE,MAP_SHARED,fd,0x56000000);
	//TODO 更改控制寄存器
	int gpfcon =(int)addr+0x50;
	*(int *)((int)addr+0x54)=0x5500;
	//0x5600 0000
#define BTN_ADDR_OFFSET 0x54
#define LED_MARK 0x0f //0000 1111b  按钮低4位
	char staut;
	while(1){
		char gpfdat=*(char*)((int)addrbtn+BTN_ADDR_OFFSET);
		char btn= LED_MARK & gpfdat;
		int i;int j=1;
		//printf("gpfdar=%02x\n",gpfdat);
		if(btn != staut)
		{
			printf("staut change\n");
			printf("%x %x\n",btn,staut);
		}
		staut=btn;
		//printf("led=%0x\n",~led<<4);
		*(char *)((int)addr+0x54)=btn<<4;
		usleep(100000);
		//*(char *)((int)addr+0x54)=~0x0f;
		//usleep(100000);
		//printf("blink\n");
	}
	//funw(fd);
	//funw(fd);
	//funr(fd);
	close(fd);
	return 0;
}
void *funw(void *st)
{
	int iret=0;
	char strin[64];
	int i;
	int fd=(*(struct sarg *)st).fd;
	int len=0;

	glen=rand()%63+1;
	for(i=0;i<glen;i++){
		*(strin+i)=rand()%26+'A';
	}
	*(strin+i)='\0';

	iret=write(fd,strin,glen+1);
	if(iret==-1){
		perror("write to kernel");
		return NULL;
	}else{
		printf("write %d :%s\n",iret,strin);
	}
	//(*(struct sarg *)st).len=iret;
	int itime=rand()%700000+300000 ;//300~1000ms
	usleep(itime);
	return ;
}
void *funr(void *st)
{
	int iret=0;
	char strout[64];
	int fd=(*(struct sarg *)st).fd;
	int len=(*(struct sarg *)st).len;
	while(1){
		iret=read(fd,strout,glen+1);
		if(iret==-1){
			perror("read from kernel");
			//continue;
		}else{
			printf("read %d :%s\n",iret,strout);
		}
		int itime=rand()%700000+300000 ;//300~1000ms
		usleep(itime);
	}
	return NULL;
}
