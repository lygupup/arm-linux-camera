#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
//#include <linux/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "cJSON.h"

#define HTTP_REQ "GET /getSysTime.do HTTP/1.1\r\nHost: quan.suning.com\r\nConnection: Close\r\n\r\n"

#define SIZE 2048

char *beijingtime=NULL;

int get_weather(char *replybuf)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket error");
		return -1;
	}
	//链接服务器
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = inet_addr("103.254.188.250");
	int ret = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if(ret < 0)
	{
		perror("链接服务器失败");
	}

	
	//发送请求
	ret = write(sockfd, HTTP_REQ, strlen(HTTP_REQ));
	if(ret <= 0)
	{
		perror("send error");
	}
	//读取服务器返回的数据
	char rbuf[SIZE]={0};
	ret = read(sockfd, rbuf, sizeof(rbuf));
	char *p = rbuf;
	while(*p != '{')
	{
		p++;
	}
	char *q = p;
	char check[5];
	strncpy(check,p,5);
	if((strncmp(check,"{\"sy",4)) ==0)	
	{
		while(*q != '}')
		{
			q++;
		}
		*(q+1) = '\0';
		strcpy(replybuf, p);
		//解析json数据
		close(sockfd);
		return 0;
	}
	else
		printf("时间api取得内容有误！正在重新获取》》》\n");
		return 2;	
}

int parse_weather(char *json)
{
	cJSON *root = cJSON_Parse(json);//创建一个解析json对象
	cJSON *result = cJSON_GetObjectItem(root,"sysTime2");
	if (!result->valuestring)
		return 0;
	else
		beijingtime = result->valuestring;

	
	/*cJSON *array = cJSON_GetObjectItem(data, "forecast");

	int count = cJSON_GetArraySize(result);

	 int i=0;
	for(i=0; i<count; i++)
	{
		cJSON *obj = cJSON_GetArrayItem(result, i);
		if(obj == NULL)
			perror("qqqq");
		//if(i == 2)
			printf("%s\n", obj->valuestring);
	} */
}

int bjtime(void)
{
	char jsonbuf[SIZE]= {0};
	re_get:
	sleep(1);
	if(get_weather(jsonbuf)==2)
		goto re_get;

	//printf("%s\n", jsonbuf);
	//解析
	parse_weather(jsonbuf);
}
