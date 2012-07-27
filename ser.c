/* filename:ser.c 27/07/12 15:51:25
   应用层.服务器.主要涉及网络编程
   通过UDP发送温度信息,
   通过TCP客户端触发,控制步进电机.
*/
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
#include <sys/socket.h>
#include <arpa/inet.h>
#define DEV_NAME "/dev/my"
#define PORT_NUMBER 5000
int main(int argc,char **argv)
{
	int fdmotor=0;int fdtemp=0;//两个文件标识.电机和温度传感器.
	int fd_skt;
	int skt_new;
	char buf[100];
	struct sockaddr_in sa;
	struct sockaddr_in sck_client;
	int ret=0;int sin_size;
//	fdmotor=open(DEV_NAME,O_RDWR);
//	if(fdmotor<0){
//		perror("open motor");
//		return -1000;
//	}
//	fdtemp=open(DEV_NAME,O_RDWR);
//	if(fdtemp<0){
//		perror("open temper");
//		return -1001;
//	}
	sa.sin_family = AF_INET;
	sa.sin_port=htons(PORT_NUMBER);
	sa.sin_addr.s_addr=inet_addr(INADDR_ANY);
	fd_skt=socket(AF_INET,SOCK_STREAM,0);
	ret=bind(fd_skt,(struct sockaddr*)&sa,sizeof(sa));
	if(ret==-1){
		perror("bind");
	}
	listen(fd_skt,SOMAXCONN);	
	while(1){
		sin_size=sizeof(struct sockaddr_in);
		skt_new=accept(fd_skt,(struct sock_addr *)&sck_client,&sin_size);
//		if(ret==-1){
//			perror("accept");
//		}
		read(skt_new,buf,100);
		buf[99]='\0';
		printf("buf=%s\n",buf);
		close(skt_new);
	}
	close(fd_skt);
	return 0;
}
