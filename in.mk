CC = gcc
CFLAGS = -g -O0 -W -Wall

COMMON_PATH = ../common
INC = -I . -I $(COMMON_PATH)

SRCS = $(wildcard ./*.c) $(wildcard $(COMMON_PATH)/*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

CURRENT_DIR = $(notdir $(shell pwd))
TARGET = $(CURRENT_DIR)

LDFLAGS = -lpthread
STATIC_LDFLAGS = -static -lpthread

define create_rootfs
    $(CC) $(CFLAGS) $(INC) $(SRCS) -o init $(STATIC_LDFLAGS)
    find init | cpio -o -Hnewc | gzip -9 > rootfs.img
endef

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

rootfs:
	$(call create_rootfs)
	qemu-system-x86_64 -kernel ../../arch/x86/boot/bzImage -initrd rootfs.img -append "console=ttyS0" -nographic

debug:
	$(call create_rootfs)
	qemu-system-x86_64 -kernel ../../arch/x86/boot/bzImage -initrd rootfs.img -append "console=ttyS0 nokaslr" -S -s -nographic

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) init rootfs.img