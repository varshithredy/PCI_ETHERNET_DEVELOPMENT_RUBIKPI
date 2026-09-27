obj-m += my_pci_stage7.o
my_pci_stage7-objs := my_pci_stage7_main.o my_pci_stage7_firmware.o my_pci_stage7_net.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
