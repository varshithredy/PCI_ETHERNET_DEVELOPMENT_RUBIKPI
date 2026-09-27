/*
 * Stage 2 - RTL8168H identification
 *
 * Stage 1 established:
 *   PCI match -> PCI enable -> MMIO BAR mapping
 *
 * Stage 2 adds only:
 *   - a simple MMIO register read
 *   - RTL8168H XID extraction
 *   - exact XID validation
 *
 * No reset, firmware, PHY, DMA, interrupt, NAPI or netdev code is
 * included yet.
 *
 * The final working P3 driver is the hardware reference. This code is
 * intentionally kept small and uses simple names so the hardware flow
 * is easy to follow.
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

static u32 read_reg32(struct my_nic *nic, u32 reg)
{
	return readl(nic->bar + reg);
}

static bool is_8168h(struct my_nic *nic)
{
	/*
	 * The working P3 driver obtains the XID from TxConfig:
	 *
	 *     xid = (TxConfig >> 20) & 0xfcf
	 *
	 * The validated RTL8168H target has XID 0x541.
	 */
	nic->tx_config = read_reg32(nic, REG_TX_CONFIG);
	nic->xid = (nic->tx_config >> 20) & 0xfcf;

	dev_info(&nic->pdev->dev,
		 "Stage 2: TxConfig=0x%08x, XID=0x%03x\n",
		 nic->tx_config, nic->xid);

	return nic->xid == 0x541;
}

static int my_pci_probe(struct pci_dev *pdev,
			const struct pci_device_id *id)
{
	struct my_nic *nic;
	unsigned long mmio_bars;
	int bar;
	int ret;

	dev_info(&pdev->dev, "Stage 2: PCI probe started\n");

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

	ret = pcim_iomap_regions(pdev, BIT(bar), "my_pci_stage2");
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
		dev_err(&pdev->dev, "MMIO BAR %d mapping returned NULL\n", bar);
		return -ENOMEM;
	}

	dev_info(&pdev->dev,
		 "Stage 2: PCI ID %04x:%04x, subsystem %04x:%04x, revision %02x\n",
		 pdev->vendor, pdev->device,
		 pdev->subsystem_vendor, pdev->subsystem_device,
		 pdev->revision);

	dev_info(&pdev->dev,
		 "Stage 2: MMIO BAR%d start=%pa size=%pa\n",
		 nic->bar_num, &nic->bar_start, &nic->bar_size);

	if (!is_8168h(nic)) {
		dev_err(&pdev->dev,
			"Stage 2: unsupported RTL8168H XID 0x%03x\n",
			nic->xid);
		return -ENODEV;
	}

	dev_info(&pdev->dev,
		 "Stage 2: RTL8168H confirmed, XID=0x%03x\n",
		 nic->xid);

	dev_info(&pdev->dev,
		 "Stage 2: probe completed successfully\n");

	return 0;
}

static void my_pci_remove(struct pci_dev *pdev)
{
	dev_info(&pdev->dev, "Stage 2: device removed\n");
}

static struct pci_driver my_pci_driver = {
	.name = "my_pci_stage2",
	.id_table = my_pci_ids,
	.probe = my_pci_probe,
	.remove = my_pci_remove,
};

module_pci_driver(my_pci_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("P3 Project");
MODULE_DESCRIPTION("Basic RTL8168H PCIe Stage 2 driver");
