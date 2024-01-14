obj-m += lpwr-int.o
SRC := $(shell pwd)

modules all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

modules_install install: all
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
