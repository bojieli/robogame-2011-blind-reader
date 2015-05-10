obj-m:=bfb.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
ifeq ($(MAKELEVEL),0)
	MODPATH=$(PWD)
else
	MODPATH=$(PWD)/bfb
endif
default:
	$(MAKE) -C $(KERNELDIR) M=$(MODPATH) modules
clean: 
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.order *.symvers
