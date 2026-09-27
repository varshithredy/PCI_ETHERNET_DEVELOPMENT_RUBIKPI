/*
 * RTL8168H P3 Driver - Stage 3
 *
 * Stage 3:
 *   PCI detection
 *   BAR2 mapping
 *   XID check
 *   basic hardware reset
 *
 * Simple return convention used in this learning/demo driver:
 *   0  = success
 *  -1  = failure
 *
 * Later stages will add MAC, firmware, PHY, DMA, interrupts,
 * NAPI and networking one step at a time.
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/delay.h>

#include "my_pci_driver.h"

static const struct pci_device_id my_pci_ids[] = {
	{
		PCI_DEVICE_SUB(RTL_VENDOR_ID, RTL_DEVICE_ID,
			       RTL_VENDOR_ID, 0x8168)
	},
	{ }
};

MODULE_DEVICE_TABLE(pci, my_pci_ids);

static u32 read_reg32(struct my_nic *nic, u32 reg)
{
	return readl(nic->mmio + reg);
}

static u8 read_reg8(struct my_nic *nic, u32 reg)
{
	return readb(nic->mmio + reg);
}

static void write_reg8(struct my_nic *nic, u32 reg, u8 value)
{
	writeb(value, nic->mmio + reg);
}

static int reset_nic(struct my_nic *nic)
{
	int count = 100;

	dev_info(&nic->pdev->dev, "Stage 3: resetting hardware\n");

	write_reg8(nic, REG_CHIP_CMD, CMD_RESET);

	while (count > 0) {
		if (!(read_reg8(nic, REG_CHIP_CMD) & CMD_RESET)) {
			dev_info(&nic->pdev->dev,
				 "Stage 3: hardware reset complete\n");
			return 0;
		}

		udelay(100);
		count--;
	}

	dev_err(&nic->pdev->dev,
		"Stage 3: hardware reset failed\n");

	return -1;
}

static int check_chip(struct my_nic *nic)
{
	u32 tx_config;
	u16 xid;

	tx_config = read_reg32(nic, REG_TX_CONFIG);
	xid = (tx_config >> 20) & 0xfcf;

	nic->xid = xid;

	dev_info(&nic->pdev->dev,
		 "Stage 3: TxConfig = 0x%08x, XID = 0x%03x\n",
		 tx_config, xid);

	if (xid != 0x541) {
		dev_err(&nic->pdev->dev,
			"Stage 3: RTL8168H XID check failed\n");
		return -1;
	}

	dev_info(&nic->pdev->dev,
		 "Stage 3: RTL8168H confirmed\n");

	return 0;
}

static int my_pci_probe(struct pci_dev *pdev,
			const struct pci_device_id *id)
{
	struct my_nic *nic;
	int ret;

	dev_info(&pdev->dev, "Stage 3: PCI probe started\n");

	nic = devm_kzalloc(&pdev->dev, sizeof(*nic), GFP_KERNEL);
	if (!nic) {
		dev_err(&pdev->dev, "Stage 3: memory allocation failed\n");
		return -1;
	}

	nic->pdev = pdev;
	pci_set_drvdata(pdev, nic);

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Stage 3: PCI enable failed\n");
		return -1;
	}

	ret = pci_request_region(pdev, 2, "my_pci_stage3");
	if (ret) {
		dev_err(&pdev->dev, "Stage 3: BAR2 request failed\n");
		pci_disable_device(pdev);
		return -1;
	}

	nic->mmio = pci_iomap(pdev, 2, 0);
	if (!nic->mmio) {
		dev_err(&pdev->dev, "Stage 3: BAR2 mapping failed\n");
		pci_release_region(pdev, 2);
		pci_disable_device(pdev);
		return -1;
	}

	dev_info(&pdev->dev,
		 "Stage 3: BAR2 mapped successfully\n");

	if (check_chip(nic) != 0)
		goto fail;

	if (reset_nic(nic) != 0)
		goto fail;

	dev_info(&pdev->dev,
		 "Stage 3: hardware ready\n");

	return 0;

fail:
	pci_iounmap(pdev, nic->mmio);
	pci_release_region(pdev, 2);
	pci_disable_device(pdev);
	return -1;
}

static void my_pci_remove(struct pci_dev *pdev)
{
	struct my_nic *nic = pci_get_drvdata(pdev);

	if (nic && nic->mmio)
		pci_iounmap(pdev, nic->mmio);

	pci_release_region(pdev, 2);
	pci_disable_device(pdev);

	dev_info(&pdev->dev,
		 "Stage 3: driver removed\n");
}

static struct pci_driver my_pci_driver = {
	.name = "my_pci_stage3",
	.id_table = my_pci_ids,
	.probe = my_pci_probe,
	.remove = my_pci_remove,
};

module_pci_driver(my_pci_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("P3 Project");
MODULE_DESCRIPTION("Basic RTL8168H PCIe Stage 3 driver");
