/*
 * Stage 1 - RTL8168H PCIe device detection
 *
 * This is intentionally a small first-stage driver.
 *
 * Stage 1 does only:
 *   1. Match the exact P3 PCI device.
 *   2. Enable the PCI device.
 *   3. Find the first MMIO BAR.
 *   4. Map that BAR.
 *   5. Print the hardware information.
 *
 * It does NOT:
 *   - reset the NIC
 *   - read the MAC address
 *   - load firmware
 *   - configure the PHY
 *   - allocate DMA rings
 *   - request interrupts
 *   - create a network interface
 *   - transmit or receive packets
 *
 * The final working RTL8168H driver supplied for this project was used
 * as the hardware/reference baseline. This stage is a separate,
 * independently structured implementation.
 */

#include "my_pci_driver.h"

static const struct pci_device_id my_pci_ids[] = {
	{
		PCI_DEVICE_SUB(MY_VENDOR_ID, MY_DEVICE_ID,
			       MY_SUBVENDOR_ID, MY_SUBDEVICE_ID)
	},
	{ }
};

MODULE_DEVICE_TABLE(pci, my_pci_ids);

static int my_pci_probe(struct pci_dev *pdev,
			const struct pci_device_id *id)
{
	struct my_nic *nic;
	unsigned long mmio_bars;
	int bar;
	int ret;

	dev_info(&pdev->dev, "Stage 1: PCI probe started\n");

	nic = devm_kzalloc(&pdev->dev, sizeof(*nic), GFP_KERNEL);
	if (!nic)
		return -ENOMEM;

	nic->pdev = pdev;
	pci_set_drvdata(pdev, nic);

	ret = pcim_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "PCI device enable failed: %d\n", ret);
		return ret;
	}

	mmio_bars = pci_select_bars(pdev, IORESOURCE_MEM);
	bar = ffs(mmio_bars) - 1;

	if (bar < 0) {
		dev_err(&pdev->dev, "No MMIO BAR found\n");
		return -ENODEV;
	}

	ret = pcim_iomap_regions(pdev, BIT(bar), "my_pci_stage1");
	if (ret) {
		dev_err(&pdev->dev, "MMIO BAR %d mapping failed: %d\n",
			bar, ret);
		return ret;
	}

	nic->bar_num = bar;
	nic->bar_start = pci_resource_start(pdev, bar);
	nic->bar_size = pci_resource_len(pdev, bar);
	nic->bar = pcim_iomap_table(pdev)[bar];

	if (!nic->bar) {
		dev_err(&pdev->dev, "MMIO BAR %d has no mapped address\n", bar);
		return -ENOMEM;
	}

	dev_info(&pdev->dev,
		 "Stage 1: RTL8168H PCI device detected\n");

	dev_info(&pdev->dev,
		 "PCI ID %04x:%04x, subsystem %04x:%04x, revision %02x\n",
		 pdev->vendor, pdev->device,
		 pdev->subsystem_vendor, pdev->subsystem_device,
		 pdev->revision);

	dev_info(&pdev->dev,
		 "MMIO BAR%d: start=%pa size=%pa mapped=%p\n",
		 nic->bar_num,
		 &nic->bar_start,
		 &nic->bar_size,
		 nic->bar);

	dev_info(&pdev->dev,
		 "Stage 1: probe completed successfully\n");

	return 0;
}

static void my_pci_remove(struct pci_dev *pdev)
{
	dev_info(&pdev->dev, "Stage 1: device removed\n");
}

static struct pci_driver my_pci_driver = {
	.name = "my_pci_stage1",
	.id_table = my_pci_ids,
	.probe = my_pci_probe,
	.remove = my_pci_remove,
};

module_pci_driver(my_pci_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("P3 Project");
MODULE_DESCRIPTION("Basic RTL8168H PCIe Stage 1 driver");
