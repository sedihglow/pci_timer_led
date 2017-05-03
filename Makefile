KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

TARGET = pciLED
pciLED-objs := pci_ledDriver.o pci_led.o 

obj-m += $(TARGET).o

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
