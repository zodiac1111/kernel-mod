#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>

#define portnumber 13333
struct cmd{
	char obj;//m / t
	signed int degree;//0~xxx
};
int main(int ac,char*av[])
{
	struct cmd cmd1;
	int sockfd;
	//char buffer[1024];
	struct sockaddr_in server_addr;
	struct hostent *host;
	/*使用hostname查询host名字*/
	if(ac!=2)
	{
		fprintf(stderr,"Usage:%s hostname\a\n",av[0]);
		exit(1);
	}
	if((host=gethostbyname(av[1]))==NULL)
	{
		fprintf(stderr,"Gethostname error\n");
		exit(1);
	}
	/*客户程序开始建立sockfd描述符*/
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)//AF_INET;SOCK_STREAM:TCP
	{
		fprintf(stderr,"socket error:%s\a\n",strerror(errno));
		exit(1);
	}
	/*客户端填充服务端的资料*/
	bzero(&server_addr,sizeof(server_addr));//初始化，置0
	server_addr.sin_family=AF_INET;     //IPV4
	server_addr.sin_port=htons(portnumber); //(将本机器上的short数据转化为网络上的short数据)端口号
	server_addr.sin_addr=*((struct in_addr*)host->h_addr); //IP地址
	/*客户端发起连接请求*/
	if(connect(sockfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"connect error:%s\a\n",strerror(errno));
		exit(1);
	}
	/*控制类型*/
	printf("Please input contrl obj m=motor t=temperature sonser:\n");
	scanf("%c",&cmd1.obj);
	fflush(stdin);
	if(cmd1.obj=='m'){//电机还有其他参数.温度传感器什么都不用了
		printf("Please input motor degree unsigned integer number\n");
		printf("signed  motor dir +=Forward -=Resaves\n");
		scanf("%d",&cmd1.degree);
	}
	write(sockfd,&cmd1,sizeof(struct cmd));
	/*通讯结束*/
	close(sockfd);
	exit(0);
}
