#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 1024*10

void show_prog(unsigned int pos, unsigned int all)
{

	unsigned int s =(int) pos/(double)all*100;

	printf("[%2d%%]\n",s);
	printf("\033[1A");//光标上偏移一行
}


int putfile(char* ip, char *port,char *filepath)
{

	//创建套接字
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("创建套接字失败");
		return -1;
	}

	//服务器端口地址
	struct sockaddr_in serveraddr;	
	//初始化结构体
	memset(&serveraddr, 0, sizeof(serveraddr));
	//初始化成员，家族协议，端口号， ip地址
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(port));  //把端口号转换为网络字节序
	serveraddr.sin_addr.s_addr = inet_addr(ip);
	//链接服务器
	int ret = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if(ret < 0)
	{
		perror("链接失败");
		return -1;
	}

	/*******************************************
	发送文件
	1，获取文件信息
	2，把文件信息发送给服务器
	3，分段读取数据并且发送（while循环发送）
	4，关闭文件
	*****************************************/

	printf("%s\n", filepath);
	//char *filepath = filepath;
	struct stat statbuf;
 	ret = stat(filepath, &statbuf);//获取文件大小
	if(ret < 0)
	{
		perror("获取信息失败");
	}
	unsigned int filesize = statbuf.st_size;//保存文件大小
	char *p = filepath;
	p += strlen(filepath);
	while(*p != '/' && p != filepath) //home/hqd/my.c
	{
		p--;
	}
	p++;
	char filename[32] = {0};
	strcpy(filename, p);//保存文件名

	//分配空间存储文件大小，文件名称
	p =(char *)malloc(4+32);
	memcpy(p, &filesize, 4);
	memcpy(p+4, filename, 32);
	write(sockfd, p, 36);//发送文件信息
	
	//打开文件
	FILE *file = fopen(filepath, "r");
	if(file == NULL)
	{
		perror("打开文件失败");
	}
	//保存读取的数据，用来发送
	char buf[SIZE] = {0};
	//保存已经发送的字节数
	unsigned int sendsize = 0;
	while(1)
	{
		ret = fread(buf, 1, SIZE, file);	
		if(ret < 0)
		{
			perror("读取失败");
			break;
		}
		if(ret == 0)
		{
			perror("读取完毕");
			break;
		}
		
		ret = write(sockfd, buf, ret);
		sendsize += ret;
		show_prog(sendsize, filesize);

	}
	fclose(file);
	return 0;
}
