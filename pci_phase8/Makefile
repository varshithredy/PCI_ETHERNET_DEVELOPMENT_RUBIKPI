obj-m += my_pci_stage8.o
my_pci_stage8-objs := my_pci_stage8_main.o my_pci_stage8_firmware.o my_pci_stage8_net.o my_pci_stage8_dma.o my_pci_stage8_phy.o
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
