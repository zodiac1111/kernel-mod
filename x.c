/* 本地应用层接收程序
 * 负责接收远程socket指令,并按照指令调用motor.ko模块 motor 运行在板子上的服务程序
 * 控制步进电机,或者读取温度传感器并返回给远程控制端.
 * */

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//监听端口
#define portnumber 13333
//监听/返回(发送)的IP
#define CTL_IP "192.168.1.200"
struct cmd{
	char obj;//m / t
	signed int degree;//0~xxx -/+ dir
};
//步进电机
int motor(struct cmd cmd1)
{

	int fd;
	int ret=0;
//	unsigned char dir=0;//方向
//	unsigned int degree=90;//角度
	fd=open("/dev/motor0",O_RDWR);
	if(fd==-1){
		perror("open ");
		return -1001;
	}
	//degree=cmd1.degree;
	if(cmd1.degree>0)//正
		ret = ioctl(fd,0,cmd1.degree);//后面参数无所谓
	else//反转
		ret = ioctl(fd,1,-cmd1.degree);
	//返回值放在errno里面了
	if(ret<0){
		printf("err %d\n", errno);
		perror("ioctl");
		return -1002;
	}
	close(fd);
	return 0;
}
//温度传感器
float ts(void)
{	
	float temper=0;
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
		close(fd);
		return -999;
	}else{
		printf("ret: %d retval=%.2f C \n",ret,ret*0.0625);
	}
	if(ret==1360){//错误的 即返回0x85
		close(fd);		
		return -999;
	}
	//usleep(100000);
	
	temper=ret*0.0625;
	return temper;
}
int main(int ac,char*av[])
{
	int sockfd,new_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t sin_size;
	int nbytes;
	struct cmd cmd1;
	char buffer[1024];
	float temper=0;
	/*服务器端开始建立sockfd描述符*/
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)//AF_INET:IPV4;SOCK_STREAM:TCP
	{
		fprintf(stderr,"socket error:%s\n\a",strerror(errno));
		exit(1);
	}
	/*服务器端填充sockaddr结构*/
	bzero(&server_addr,sizeof(struct sockaddr_in));//初始化，置0
	server_addr.sin_family=AF_INET;//Internet
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);//（将本机器上的long数据转化成网络上的long数据）和任何主机通信//INADDR_ANY表示可以接收任意IP地址的数据，即绑定到所有的IP
	//server_addr.sin_addr.s_addr=inet_addr("192.168.1.1");//用于绑定到一个固定IP，inet_addr用于把数字加格式的ip转化为整形ip
	server_addr.sin_port=htons(portnumber);//(将本机器上的short数据转化为网络上的short数据)端口号
	/*捆绑sockfd描述符到IP地址*/
	if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
		exit(1);
	}
	/*设置允许连接的最大客户端数*/
	if(listen(sockfd,5)==-1)
	{
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
		exit(1);
	}
	while(1)
	{
		/*服务器阻塞，直到客户端程序建立连接*/
		sin_size=sizeof(struct sockaddr_in);
		if((new_fd=accept(sockfd,(struct sockaddr*)&client_addr,&sin_size))==-1)
		{
			fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
			exit(1);
		}
		fprintf(stderr,"Server get connect from %s\n",inet_ntoa(client_addr.sin_addr));//将网络地址转化成字符串
		if((nbytes=read(new_fd,buffer,sizeof(struct cmd)))==-1)
		{
			fprintf(stderr,"Read Error:%s\n",strerror(errno));
			exit(1);
		}
		printf("nbyte=%d",nbytes);
		if(sizeof(struct cmd)!=nbytes){
			printf("return size err");
			continue;
		}
		//buffer[nbytes]='\0';//使得字符串数组的最后一个元素为结束符
		memcpy(&cmd1,buffer,sizeof(struct cmd));
		printf("server received cmd obj=%c\n",cmd1.obj);
		
		if(cmd1.obj=='m'){//m 电动机
			printf("degree= %d\n",cmd1.degree);
			motor(cmd1);	
		}else{
			temper=ts();
			//char test[4]={0x41,0xf9,0x80,0x00};
			//char *test
			//test[0]=temper/10+'0';
			//test[0]=temper/10+'0';			
			//(char [4])temper;
			//int i=(int)temper;
			write(new_fd,&temper,sizeof(temper));
			//write(new_fd,test,sizeof(test));		
		}
		/*这个通讯已经结束*/
		close(new_fd);
		/*循环下一个*/
	}
	/*结束通讯*/
	close(sockfd);
	exit(0);
}
