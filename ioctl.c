/*简单ioctl程序
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
struct st{
	int len;
	char *p;
};
int main(int argc,char* argv[])
{
	int fd;
	char *ptr="zodiac\n";
	int cmd=4;
	int ret=0;
	int arg=0;
	struct st st1;
	if(argc!=3){
		printf("arg c must be 3\n"
				"usage:ioctl <cmd> <arg>\n");
		return -1;
	}
	fd=open("/dev/my",O_RDWR);
	if(fd==-1){
		perror("open ");
	}else{
		printf("fd is %d\n",fd);
		usleep(100000);
	}
	cmd=strtoul(argv[1],0,10);
	st1.len=strlen(argv[2])+1;
	st1.p=argv[2];
	//ret =ioctl(fd,cmd,&st1);
	//for led ctl
	arg=strtoul(argv[2],0,10);
	ret =ioctl(fd,cmd,arg);
	usleep(100000);
	//返回值放在errno里面了
	printf("err %d\n", errno);
	printf("ret: %d cmd %d, ptr %s\n",ret,cmd,ptr);
	usleep(100000);
	close(fd);
	return 0;
}
