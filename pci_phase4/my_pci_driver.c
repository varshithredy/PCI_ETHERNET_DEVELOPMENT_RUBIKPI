/*
 * P3 PCIe Ethernet Driver - Stage 4
 *
 * Stage 4 adds:
 *   - reading the MAC address
 *   - checking that the MAC address looks valid
 *
 * Earlier stages already do:
 *   - PCI detection
 *   - BAR2 mapping
 *   - chip identification
 *   - hardware reset
 *
 * This driver intentionally uses simple names.
 * At this stage we are treating the hardware simply as:
 *
 *       PCI device
 *          |
 *          +-- registers
 *          |
 *          +-- MAC address
 *
 * We are not using names from the original Realtek driver here.
 *
 * No firmware, PHY, DMA, interrupts, NAPI or network interface yet.
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>

#include "my_pci_driver.h"

static const struct pci_device_id my_pci_ids[] = {
	{
		PCI_DEVICE_SUB(DEVICE_VENDOR_ID, DEVICE_ID,
			       DEVICE_SUBVENDOR, DEVICE_SUBDEVICE)
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

static int check_device(struct my_nic *nic)
{
	nic->tx_config = read_reg32(nic, REG_TX_CONFIG);
	nic->xid = (nic->tx_config >> 20) & 0xfcf;

	dev_info(&nic->pdev->dev,
		 "Stage 4: TxConfig = 0x%08x, ID = 0x%03x\n",
		 nic->tx_config, nic->xid);

	if (nic->xid != 0x541) {
		dev_err(&nic->pdev->dev,
			"Stage 4: device check failed\n");
		return -1;
	}

	dev_info(&nic->pdev->dev,
		 "Stage 4: device check passed\n");

	return 0;
}

static int reset_device(struct my_nic *nic)
{
	int count = 100;

	dev_info(&nic->pdev->dev,
		 "Stage 4: resetting device\n");

	write_reg8(nic, REG_COMMAND, RESET_BIT);

	while (count > 0) {
		if (!(read_reg8(nic, REG_COMMAND) & RESET_BIT)) {
			dev_info(&nic->pdev->dev,
				 "Stage 4: reset complete\n");
			return 0;
		}

		udelay(100);
		count--;
	}

	dev_err(&nic->pdev->dev,
		"Stage 4: reset failed\n");

	return -1;
}

static int read_mac(struct my_nic *nic)
{
	int i;

	for (i = 0; i < ETH_ALEN; i++)
		nic->mac[i] = read_reg8(nic, MAC_REG + i);

	if (!is_valid_ether_addr(nic->mac)) {
		dev_err(&nic->pdev->dev,
			"Stage 4: invalid MAC address\n");
		return -1;
	}

	dev_info(&nic->pdev->dev,
		 "Stage 4: MAC address %pM\n",
		 nic->mac);

	return 0;
}

static int my_probe(struct pci_dev *pdev,
		    const struct pci_device_id *id)
{
	struct my_nic *nic;
	int ret;

	dev_info(&pdev->dev,
		 "Stage 4: probe started\n");

	nic = devm_kzalloc(&pdev->dev,
			   sizeof(*nic),
			   GFP_KERNEL);

	if (!nic) {
		dev_err(&pdev->dev,
			"Stage 4: memory allocation failed\n");
		return -1;
	}

	nic->pdev = pdev;
	pci_set_drvdata(pdev, nic);

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev,
			"Stage 4: PCI enable failed\n");
		return -1;
	}

	ret = pci_request_region(pdev, 2, "my_pci_stage4");
	if (ret) {
		dev_err(&pdev->dev,
			"Stage 4: BAR2 request failed\n");
		pci_disable_device(pdev);
		return -1;
	}

	nic->mmio = pci_iomap(pdev, 2, 0);

	if (!nic->mmio) {
		dev_err(&pdev->dev,
			"Stage 4: BAR2 mapping failed\n");
		pci_release_region(pdev, 2);
		pci_disable_device(pdev);
		return -1;
	}

	dev_info(&pdev->dev,
		 "Stage 4: BAR2 mapped\n");

	if (check_device(nic) != 0)
		goto fail;

	if (reset_device(nic) != 0)
		goto fail;

	if (read_mac(nic) != 0)
		goto fail;

	dev_info(&pdev->dev,
		 "Stage 4: device is ready with MAC %pM\n",
		 nic->mac);

	return 0;

fail:
	pci_iounmap(pdev, nic->mmio);
	pci_release_region(pdev, 2);
	pci_disable_device(pdev);

	return -1;
}

static void my_remove(struct pci_dev *pdev)
{
	struct my_nic *nic = pci_get_drvdata(pdev);

	if (nic && nic->mmio)
		pci_iounmap(pdev, nic->mmio);

	pci_release_region(pdev, 2);
	pci_disable_device(pdev);

	dev_info(&pdev->dev,
		 "Stage 4: driver removed\n");
}

static struct pci_driver my_driver = {
	.name = "my_pci_stage4",
	.id_table = my_pci_ids,
	.probe = my_probe,
	.remove = my_remove,
};

module_pci_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("P3 Project");
MODULE_DESCRIPTION("Basic PCIe Ethernet Driver - Stage 4");
