obj-m += my_pci_stage9.o

my_pci_stage9-objs := my_pci_stage9_main.o \
                      my_pci_stage9_firmware.o \
                      my_pci_stage9_net.o \
                      my_pci_stage9_dma.o \
                      my_pci_stage9_phy.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
