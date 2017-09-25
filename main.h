#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> //perror ,errno
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <linux/input.h> //struct input_event
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "api_v4l2.h"
#include "jpeglib.h"
#include <stdio.h>
extern char *beijingtime;
int putfile(char* ip, char *port,char *filepath);
void printfgb2312(unsigned char *str,int x,int y);
void gb2312_run(unsigned char *str,int x,int y);
void gb2312_run1(unsigned char *str,int x,int y);
void asc(unsigned char *str);
int bjtime(void);
void init_avi(void);
void close_avi(void);
int process_image(void *addr, int length);


int *plcd = NULL;
int off_view=0;
int pause_view=0;
int v_on=0;
int num_name=0;
char *aviname;
//extern char *systime;
//int vfd;
FreamBuffer freambuf;


enum finger_touch
{
	START_VIEW=1,
	STOP_VIEW,
	GET_PHOTO,
	VIDEO_RECORD,
};

void *run(void *arg)
{
	pthread_detach(pthread_self());
	if (((int)arg)==1)
		gb2312_run1("正在录像！！！",0,232);

	else
		gb2312_run("欢迎使用本摄像头！",0,420);	
}
void lcd_draw_point(int x, int y, int color)
{
	int *p = plcd;
	*(p + y*800 + x) = color;
}

void lcd_clear_screen(int color)
{
	int x, y;
	for (y = 0; y < 480; y++)
	{
		for (x = 0; x < 800; x++)
			lcd_draw_point(x, y, color);
	}
}

void lcd_draw_rect(int x0, int y0, int w, int h, int color)
{
	int x, y;
	for( y = y0; y < (y0+h); y++)
	{
		for (x = x0; x < (x0+w); x++)
			lcd_draw_point(x, y, color);
	}
}

void show_jpeg(char *path/*struct LcdDev* lcd, unsigned char *jpegbuf, unsigned long len*/)
{	
	//1.定义解码对象，错误处理对象
	struct  jpeg_decompress_struct info;
	struct  jpeg_error_mgr mgr;
	int *jpeq=plcd;
	//2.绑定错误处理
	info.err = jpeg_std_error(&mgr);
#if 1
	//3.打开要解码的文件
	FILE *file = fopen(path,"rb");
	if(file ==  NULL) perror("fopen error");
#endif
	//4.初始化解码对象
	jpeg_create_decompress(&info);
#if 1
	//5.关联数据（导入数据）
	jpeg_stdio_src(&info, file);
#else
	jpeg_mem_src(&info, jpegbuf, len);
#endif
	//6.读取jpeg文件头信息
	jpeg_read_header(&info, TRUE);

	//7.开始解码
	jpeg_start_decompress(&info);

	//8.分配空间存储一行数据（解码好的数据）
	unsigned int rowsize = info.output_width * info.output_components;
	unsigned char *mp = malloc(rowsize);
	//unsigned int *p = (unsigned int *)(((unsigned char *)lcd->mp)+1);

	//printf("%d:%d:%d\n", info.output_width, info.output_height, info.output_components);
	//9.一行一行读取数据
	
	while(info.output_scanline < info.output_height)
	{
		jpeg_read_scanlines(&info, &mp, 1);//解码对象， 存储数据的地址，最大行数

		int i=0;
		unsigned int data=0;
		unsigned char *tp = mp;
		for(i=0; i<info.output_width; i++)
		{
			//argb argb argb rgb rgb rbg
			//memcpy(lcd->mp+(info.output_scanline-1)*800+i, mp+i*3, 3);
			data |= tp[0]<<16;
			data |= tp[1]<<8;	
			data |= tp[2];	
			tp+=3;
			*(jpeq+(info.output_scanline-1)*800+i) = data;
			data=0;
		}
	}

	//10.解码完成
	jpeg_finish_decompress(&info);
	//11.释放空间
	jpeg_destroy_decompress(&info);
	free(mp);
}
int get_finger_touch_direction()
{
	int fdev, r;
	int x, y;
	fdev = open("/dev/input/event0", O_RDONLY);
	if (fdev == -1)
	{
	printf("open event0 error !\n");
	return -1;
	}
	struct input_event ev;
	while (1)
	{
		r = read(fdev, &ev, sizeof(ev));
		if (r != sizeof(ev))
			continue;

		if ((ev.type == EV_ABS) &&(ev.code == ABS_X))
			x = ev.value;	
		if ((ev.type == EV_ABS) && (ev.code == ABS_Y))
			y = ev.value;	

		if (x>660&&x<780)
		{
		
			if (y < 230)
			{
				if (y > 90&&y < 140)
				{printf("START_VIEW\n");
				close(fdev);
				return START_VIEW;
				}
				else if (y > 180)
				{
				printf("STOP_VIEW\n");
				close(fdev);
				return STOP_VIEW;
				}
			}
			else if (y > 270)
			{
				if (y < 320)
				{
				printf("GET_PHOTO\n");
				close(fdev);
				return GET_PHOTO;
				}
				else if (y > 360&&y < 410)
				{
				printf("VIDEO_RECORD\n");
				close(fdev);
				return VIDEO_RECORD; 
				}
			}
			else
			{
				x = -1;
				y = -1;
			}
		}
	}
	close(fdev);
	return 0;
}
void *handle_video_record(void *arg)
{
	pthread_detach(pthread_self());
	v_on+=1;
	if((v_on%2)==1)
	{	
		aviname=beijingtime;
		pthread_t v6;
		void *arg6;
		pthread_create(&v6,NULL,run,(void *)1);
		lcd_draw_rect(660,360,120,50,0x559400D3);
		printfgb2312("录像",680,370);
		printf("start avi write\n");
		init_avi();
	}
	else
	{	
		usleep(300000);
		close_avi();
		lcd_draw_rect(660,360,120,50,0x55171717);
		printfgb2312("录像",680,370);
		char temp[64]={0};
		sprintf(temp,"./%s.avi",aviname);
		putfile("192.168.5.24","9688",temp);
		printf("avi write finished\n");
	}
}
void *handle_get_photo(void *arg)
{
	pthread_detach(pthread_self());
	pause_view=1;
	char buf[16]={0};
	sprintf(buf,"./%d.jpg",num_name++);
	int fd = open(buf, O_CREAT|O_RDWR,0777);
	
	int ret = write(fd, freambuf.buf, freambuf.length);
	putfile("192.168.5.24","9688",buf);
	close(fd);
	sleep(1);
	pause_view=0;
}
void *handle_stop_view(void *arg)
{
	pthread_detach(pthread_self());
	off_view=1;
	sleep(1);
	off_view=0;
}

void *runtime(void *arg)
{
	pthread_detach(pthread_self());
	while(1)
	{
		bjtime();
		asc(beijingtime);
	}	
}
void *handle_start_view(void *arg)
{
	pthread_detach(pthread_self());
	pthread_t v5;
	void *arg5;
	pthread_create(&v5,NULL,run,arg5);
	usleep(10000);
	linux_v4l2_device_init("/dev/video0");	
	
	
	linux_v4l2_start_capturing();


	
	int *p;
	while(1)
	{
		if(off_view==1)
		{

			break;
		}	
		if(pause_view==1)
			sleep(1);
		p = plcd;
		p+= 0;
		usleep(100);
		linux_v4l2_get_fream(&freambuf);	
		
	
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		
		cinfo.err = jpeg_std_error(&jerr);		
	
		jpeg_create_decompress(&cinfo);
	
		jpeg_mem_src(&cinfo,freambuf.buf,freambuf.length);
		
		jpeg_read_header(&cinfo, TRUE);//640*480=4:3
		//cinfo.scale_num=2;
		//cinfo.scale_denom =1;
		//		
		jpeg_start_decompress(&cinfo);
				
		//printf("width = %d  height=%d\n", cinfo.output_width, cinfo.output_height);
		if((v_on%2)==1)
			process_image((void*)(freambuf.buf),(int)(freambuf.length));	

		unsigned char *buf = malloc(cinfo.output_width*cinfo.output_components);//640*3=
		unsigned char *buffer = buf;
			
		while(cinfo.output_scanline < cinfo.output_height )
		{
			
			//printf("%d\n", count++);
			//printf("output_scanlines =%d\n", cinfo.output_scanline);
			buf = buffer;
			jpeg_read_scanlines(&cinfo,&buf,1);
			
			
			int i=0;
			unsigned int data =0;
			for(i=0; i<cinfo.output_width; i++)
			{
				data = *(buf+2);
				data = data | *(buf+1)<<8;
				data = data | *(buf)<<16;
				
				*(p+i) = data;
				buf+=3;
			}
			p += 800;
		}
		
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);	
		free(buffer);
	}
	linux_v4l2_stop_capturing();
	linux_v4l2_device_uinit("/dev/video0");
}

