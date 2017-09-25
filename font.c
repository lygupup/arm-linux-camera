#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

extern int *plcd;
extern int off_view;
extern int pause_view;
extern int v_on;
void lcd_draw_rect(int x0, int y0, int w, int h, int color);
void show_jpeg(char *path);
/*
函数功能：获取一个汉字的点阵
参数：points 存储点阵
	  fontsize汉字大小
	  str汉字
	  fontpath字库路径
*/
int get_font_points(unsigned char *points, int fontsize, unsigned char *str, const char *fontpath)
{
	if(points == NULL) 
	{
		perror("points empty");
		return -1;		
	}
	
	//2.打开字库文件
	FILE *file = fopen(fontpath, "r");
	//3.字库偏移量offset=(94*(区码-1)+(位码-1))*一个汉字所占用的字节数 
	unsigned int offset = ( 94*((str[0]-0xa0)-1) + ((str[1]-0xa0)-1) ) * (fontsize/8)*fontsize;
	
	//printf("%x:%x--%d\n", str[0], str[1], offset);
	fseek(file, offset, SEEK_SET);
	
	int ret = fread(points, (fontsize/8)*fontsize, 1, file);
	if(ret < 0)
	{
		perror("fread fail");
		fclose(file);
		return -1;
	}
	fclose(file);
	return 0;
}

/*
函数功能：在终端或者是lcd上绘制点阵
参数：lcd如果lcd不为NULL就把points的点绘制在lcd上，如果为空就输出在终端
	  points点阵
	  fontsize字大小
*/

void draw_point( int *lcd, int x, int y, unsigned char *points,int fontsize)
{
	int row=0, type=0;
	unsigned int *p = (lcd+y*800+x);
	for(row=0; row<fontsize; row++)//汉字点阵第几行
	{
		for(type=0; type<fontsize/8; type++)//每一行第几个字节
		{
			unsigned char ch = points[row*fontsize/8+type];
			int i=0;
			for(i=7; i>=0; i--) //每一个字节的第几位
				if(ch&(1<<i)&&(pause_view==0))

					*(p+row*800+type*8+7-i) = 0x00ffff; 							
		}		
	}	
}


void printfgb2312(unsigned char *str,int x,int y)
{
	
	unsigned int size = 16;
	unsigned char points[size/8*size];
	memset(points, 0,sizeof(points));
	
	int len = strlen(str);
	//printf("num=%d\n", len/2);
	int i=0;
	for(i=0; i<len/2; i++)
	{
		unsigned char buf[3]={0};
		strncpy(buf, str+i*2, 2);
		int ret = get_font_points(points, size, buf, "./HZK16.dzk");
		if(ret < 0)
			perror("gb2312 !!!");	
		draw_point(plcd, i*size+x, size+y, points,  size);
	}		
}
void gb2312_run(unsigned char *str,int x,int y)
{
	
	unsigned int size = 16;
	unsigned char points[size/8*size];
	memset(points, 0,sizeof(points));
	
	int len = strlen(str);
	//printf("num=%d\n", len/2);
	int i=0;
	int t=0;
	while(1)
	{
		if(off_view==1)
		{
			usleep(300000);
			lcd_draw_rect(0,0,640,480,0x55555555);
			show_jpeg("linuxlogo.jpg");
			break;
		}	
		for(i=0; i<len/2; i++)
		{
			
			
			unsigned char buf[3]={0};
			strncpy(buf, str+i*2, 2);
			int ret = get_font_points(points, size, buf, "./HZK16.dzk");
			if(ret < 0)
				perror("gb2312 !!!");	
			draw_point(plcd, i*size+t*1/60+x, size+y, points,  size);
		}
		t++;
		usleep(1000);
		if (t==24000)
			t=0;	
	}
}
void gb2312_run1(unsigned char *str,int x,int y)
{
	
	unsigned int size = 16;
	unsigned char points[size/8*size];
	memset(points, 0,sizeof(points));
	
	int len = strlen(str);
	//printf("num=%d\n", len/2);
	int i=0;
	int t=0;
	while(1)
	{
		if((v_on%2)!=1)
		{
			usleep(30000);
			break;
		}		
		for(i=0; i<len/2; i++)
		{
			
			
			unsigned char buf[3]={0};
			strncpy(buf, str+i*2, 2);
			int ret = get_font_points(points, size, buf, "./HZK16.dzk");
			if(ret < 0)
				perror("gb2312 !!!");	
			draw_point(plcd, i*size+t*1/60+x, size+y, points,  size);
		}
		t++;
		usleep(1000);
		if (t==24000)
			t=0;	
	}
}
void oneascbuf(unsigned char *Get_Input_Char,int n)
{
	char buff[16]={0};
	
	unsigned long offset;
	FILE *ASC;
	/*打开字库文件asc16*/
	if ((ASC = fopen("ASC16", "rb+")) == NULL)
	{
		printf("Can't open asc,Please add it?");
		system("pause");
		exit(0);
	}
	offset = *(Get_Input_Char) * 16 + 1;         /*通过ascii码算出偏移量*/
	fseek(ASC, offset, SEEK_SET);                /*将文件指针移动到偏移量的位置*/
	fread(buff, 16, 1, ASC);                     /*从偏移量的位置读取32个字节*/
	//printf("ASCII:%d,offset:%d \n\r", *Get_Input_Char, offset);
	fclose(ASC);

	int i, j;
	for (i = 0; i < 16; i++)          /* 8x16的点阵，一共有16行*/
	{
		for (j = 0; j < 8; j++)         /*横向一个字节8位，依次判断每位是否为0*/
			if (buff[i] & (0x80 >> j))   /*测试当前位是否为1*/
				*(plcd+i*800+j+(8*n)+640) = 0x00ffff; /*为1的显示为字符c1*/
			else     
				*(plcd+i*800+j+(8*n)+640) = 0x55555555;
	}
}
void asc(unsigned char *str)
{		
	int len = strlen(str);
	int i=0;
	for(i=0; i<len; i++)
	{
		oneascbuf(str+i,i);
	}					
}