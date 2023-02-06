obj-m += module_ssd1306.o driver_ssd1306.o driver_ssd1306_interface.o driver_ssd1306_basic.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean