#ifndef MY_PCI_DRIVER_H
#define MY_PCI_DRIVER_H

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>

#define MY_VENDOR_ID       0x10ec
#define MY_DEVICE_ID       0x8161
#define MY_SUBVENDOR_ID    0x10ec
#define MY_SUBDEVICE_ID    0x8168

struct my_nic{
	struct pci_dev *pdev;
	void __iomem *bar;
	int bar_num;
	resource_size_t bar_start;
	resource_size_t bar_size; };

#endif
