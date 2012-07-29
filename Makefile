LINUX_KERNEL_DIR =/home/zodiac1111/Downloads/techer-ftp/devboard/kernel/utu-Linux2.6.24_for_hw2440_2012-06-14
#LINUX_KERNEL_DIR =/home/zodiac1111/Downloads/linux-3.4.4
PRODUCT_DRIVER_DIR = `pwd`
obj-m += mem_test.o
obj-m += do.o
obj-m += ts.o
obj-m += motor.o
all:  plat module app wdt ioctl 18b20
#	make module
#	make app
#	make wdt
#
clear:
	clear
module:mem_test.c do.c
	make -C $(LINUX_KERNEL_DIR) \
		SUBDIRS=$(PRODUCT_DRIVER_DIR) modules
	cp mem_test.ko do.ko /home/zodiac1111/tftpboot
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
#
18b20:18b20.c
	arm-linux-gcc 18b20.c -o 18b20 -Wall
	cp 18b20 /home/zodiac1111/tftpboot  
plat:
	make -C api 
#温度传感器 和实例程序
ts:ts.c ts-ctl.c
	make -C $(LINUX_KERNEL_DIR) \
		SUBDIRS=$(PRODUCT_DRIVER_DIR) modules
	arm-linux-gcc ts-ctl.c -o ts-ctl -Wall
	cp ts.ko /home/zodiac1111/tftpboot
	cp ts-ctl /home/zodiac1111/tftpboot
motor:motor.c motor-ctl.c
	make -C $(LINUX_KERNEL_DIR) \
		SUBDIRS=$(PRODUCT_DRIVER_DIR) modules
	arm-linux-gcc motor-ctl.c -o motor-ctl -Wall
	cp motor.ko /home/zodiac1111/tftpboot
	cp motor-ctl /home/zodiac1111/tftpboot
clean:
	rm -R -rf *.ko  *.o *.swp *.cmd app a.out wdt *~ ioctl 18b20 ts-ctl ser ds18b20 motor
