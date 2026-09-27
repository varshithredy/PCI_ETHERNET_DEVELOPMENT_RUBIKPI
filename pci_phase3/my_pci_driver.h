#ifndef MY_PCI_DRIVER_H
#define MY_PCI_DRIVER_H

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>

#define RTL_VENDOR_ID 0x10ec
#define RTL_DEVICE_ID 0x8161

#define REG_CHIP_CMD   0x37
#define REG_TX_CONFIG  0x40

#define CMD_RESET      0x10

struct my_nic {
	struct pci_dev *pdev;
	void __iomem *mmio;
	u32 xid;
};

#endif
