/* 应用层步进电机控制实例
 * 
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
#include "motor_dir.h"
//define in motor_dir.h
//#define IOCMD_MOTOR_DIR_FORWARD 0	//电机正转
//#define IOCMD_MOTOR_DIR_REVERSE 1	//电机反转
int main(int argc,char* argv[])
{
	int fd;
	int ret=0;
	unsigned char dir=0;//方向
	unsigned int degree=90;//角度
	if(argc!=3){
		printf("Usage:motor-ctl <dir 0:Forward | 1:Reverse> <degree>\n");
		printf("use default param: dir=0[Forward] degree=90\n");
		//dir=0;degree=90;		
	}else{
		dir=strtoul(argv[1],0,10);
		degree=strtoul(argv[2],0,10);	
	}
	fd=open("/dev/motor0",O_RDWR);
	if(fd==-1){
		perror("open ");
		return -1001;
	}
	ret = ioctl(fd,dir,degree);//后面参数无所谓
	//返回值放在errno里面了
	if(ret<0){
		printf("err %d\n", errno);
		perror("ioctl");
		return -1002;
	}
	//printf("ret: %d retval=%.2f C \n",ret,ret*0.0625);
	//usleep(100000);
	close(fd);
	return 0;
}
