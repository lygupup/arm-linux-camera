#include "main.h"


int main(int argc, char *argv[])
{
	int fd;
	
	fd = open("/dev/fb0", O_RDWR);
	//vfd = open("/dev/video0", O_RDONLY);
	if (fd == -1)
	{
		printf("open lcd error !\n");
		return 0;
	}
	
	plcd = mmap(NULL, 800*480*4,PROT_WRITE,MAP_SHARED, fd, 0);
	lcd_clear_screen(0x55555555);
	show_jpeg("linuxlogo.jpg");

	//lcd_draw_rect(660,0,120,50,0x111111);
	lcd_draw_rect(660,90,120,50,0x55171717);	
	lcd_draw_rect(660,180,120,50,0x55171717);
	lcd_draw_rect(660,270,120,50,0x55171717);	
	lcd_draw_rect(660,360,120,50,0x55171717);
	printfgb2312("¿ªÊ¼Ô¤ÀÀ",680,100);
	printfgb2312("¹Ø±ÕÔ¤ÀÀ",680,190);
	printfgb2312("ÅÄÕÕ±£´æ",680,280);
	printfgb2312("Â¼Ïñ",680,370);
	//bjtime();
		//asc(beijingtime);
	pthread_t v6;
	void *arg6;
	pthread_create(&v6,NULL,runtime,arg6);
	
	while (1)
	{
		int figer;
		figer = get_finger_touch_direction();
		if (figer == START_VIEW)
		{
			usleep(10000);
			pthread_t v1;
			void *arg1;
			pthread_create(&v1,NULL,handle_start_view,arg1);
			usleep(100000);
		
		}
		else if (figer == STOP_VIEW)
		{
			usleep(10000);
			pthread_t v2;
			void *arg2;
			pthread_create(&v2,NULL,handle_stop_view,arg2);
			usleep(100000);
		
		}
		else if (figer == GET_PHOTO)
		{
			usleep(10000);
			pthread_t v3;
			void *arg3;
			pthread_create(&v3,NULL,handle_get_photo,arg3);
			usleep(100000);
			
		}
		else if (figer == VIDEO_RECORD)
		{
			usleep(10000);
			pthread_t v4;
			void *arg4;
			pthread_create(&v4,NULL,handle_video_record,arg4);
			usleep(100000);
		}
	}
	munmap(plcd, 800*480*4);
	close(fd);
	return 0;
}