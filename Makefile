LINUX_KERNEL_DIR =/home/zodiac1111/Downloads/techer-ftp/devboard/kernel/utu-Linux2.6.24_for_hw2440_2012-06-14
#LINUX_KERNEL_DIR =/home/zodiac1111/Downloads/linux-3.4.4
PRODUCT_DRIVER_DIR = `pwd`
obj-m += mem_test.o
all:module app wdt ioctl
#	make module
#	make app
#	make wdt
#
module:mem_test.c
	make -C $(LINUX_KERNEL_DIR) \
		SUBDIRS=$(PRODUCT_DRIVER_DIR) modules
	cp mem_test.ko /home/zodiac1111/tftpboot
#app
app:app.c
	@echo "***** cp to tftpboot ******"
	arm-linux-gcc app.c -o app -lpthread -Wall
	cp app /home/zodiac1111/tftpboot
wdt:wdt.c
	arm-linux-gcc wdt.c -o wdt -Wall
	cp wdt /home/zodiac1111/tftpboot
#
ioctl:ioctl.c
	arm-linux-gcc ioctl.c -o ioctl -Wall
	cp ioctl /home/zodiac1111/tftpboot  
clean:
	rm -rf *.ko *.o *.swp *.cmd app a.out wdt
