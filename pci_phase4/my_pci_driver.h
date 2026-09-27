#ifndef MY_PCI_DRIVER_H
#define MY_PCI_DRIVER_H

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/etherdevice.h>

#define DEVICE_VENDOR_ID   0x10ec
#define DEVICE_ID          0x8161
#define DEVICE_SUBVENDOR   0x10ec
#define DEVICE_SUBDEVICE  0x8168

#define REG_COMMAND        0x37
#define REG_TX_CONFIG      0x40

#define RESET_BIT          0x10

/* The first six bytes of the device register space contain the MAC. */
#define MAC_REG            0x00

struct my_nic {
	struct pci_dev *pdev;
	void __iomem *mmio;

	u32 tx_config;
	u16 xid;

	u8 mac[ETH_ALEN];
};

#endif
