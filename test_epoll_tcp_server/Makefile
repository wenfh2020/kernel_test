CC = gcc
CFLAGS = -g -O0

SRCS = $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))
TARGET = et

.PHONY: clean

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

rootfs:
	gcc -o init $(SRCS) -m32 -static -lpthread -Wall
	find init | cpio -o -Hnewc |gzip -9 > ../rootfs.img
	qemu-system-x86_64 -kernel ../../arch/x86/boot/bzImage -initrd ../rootfs.img

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) init