SRCS=$(wildcard *.c */*.c */*/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
CC=arm-linux-gnueabi-gcc
TARGET=main
CONFIG=-ljpeg -lpthread -lm

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CONFIG)

%.o:%.c 
	$(CC) -o $@ -c $< -I./
clean:
	rm $(OBJS) $(TARGET)
