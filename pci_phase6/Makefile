obj-m += my_pci_stage6.o
my_pci_stage6-objs := my_pci_stage6_main.o my_pci_stage6_fw.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
