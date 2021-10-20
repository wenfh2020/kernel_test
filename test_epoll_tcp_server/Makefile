CC = gcc
CFLAGS = -g -O0 -W  -Wall

COMMON_PATH = ../common
INC = -I . -I $(COMMON_PATH)

SRCS = $(wildcard ./*.c) $(wildcard $(COMMON_PATH)/*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

CURRENT_DIR = $(notdir $(shell pwd))
TARGET = $(CURRENT_DIR)

.PHONY: clean

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

rootfs:
	$(CC) $(CFLAGS) $(INC) $(SRCS) -o init -static -lpthread
	find init | cpio -o -Hnewc | gzip -9 > ../rootfs.img
	qemu-system-x86_64 -kernel ../../arch/x86/boot/bzImage -initrd ../rootfs.img

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) init ../rootfs.img