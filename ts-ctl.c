/* 应用层温度传感器调用实例
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
int main(int argc,char* argv[])
{
	int fd;
	int ret=0;
	fd=open("/dev/ts0",O_RDWR);
	if(fd==-1){
		perror("open ");
		return -1001;
	}
	ret = ioctl(fd,0,0);//后面参数无所谓
	//返回值放在errno里面了
	if(ret<0){
		printf("err %d\n", errno);
		perror("ioctl");
		return -1002;
	}else{
		printf("ret: %d retval=%.2f C \n",ret,ret*0.0625);
	}
	//usleep(100000);
	close(fd);
	return 0;
}
