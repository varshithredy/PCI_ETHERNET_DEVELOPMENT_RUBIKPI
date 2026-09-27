#include "my_pci_stage7.h"

static const struct pci_device_id device_ids[] = {
    { PCI_DEVICE_SUB(DEVICE_VENDOR_ID, DEVICE_ID,
                     DEVICE_SUBVENDOR, DEVICE_SUBDEVICE) },
    { }
};
MODULE_DEVICE_TABLE(pci, device_ids);

u32 mac_reg_read(struct my_nic *nic, u32 reg)
{
    return readl(nic->mmio + reg);
}

void mac_reg_write(struct my_nic *nic, u32 reg, u32 value)
{
    writel(value, nic->mmio + reg);
}

static int wait_access(struct my_nic *nic, u32 reg, bool busy)
{
    int i;

    for (i = 0; i < 100; i++) {
        u32 value = mac_reg_read(nic, reg);
        if (!!(value & ACCESS_BUSY) == busy)
            return 0;
        udelay(10);
    }
    return -1;
}

int phy_write(struct my_nic *nic, u16 reg, u16 value)
{
    u32 address;

    if (reg == 0x1f) {
        nic->ocp_base = value ? ((u32)value << 4) : OCP_STD_PHY_BASE;
        return 0;
    }

    if (nic->ocp_base != OCP_STD_PHY_BASE) {
        if (reg < 0x10)
            return -1;
        reg -= 0x10;
    }

    address = nic->ocp_base + ((u32)reg * 2);
    if (address & 0xffff0001U)
        return -1;

    mac_reg_write(nic, REG_GPHY_OCP,
                  ACCESS_BUSY | (address << 15) | value);
    return wait_access(nic, REG_GPHY_OCP, false);
}

int phy_read(struct my_nic *nic, u16 reg, u16 *value)
{
    u32 address;

    if (!value)
        return -1;

    if (reg == 0x1f) {
        *value = nic->ocp_base == OCP_STD_PHY_BASE ?
                 0 : (u16)(nic->ocp_base >> 4);
        return 0;
    }

    if (nic->ocp_base != OCP_STD_PHY_BASE) {
        if (reg < 0x10)
            return -1;
        reg -= 0x10;
    }

    address = nic->ocp_base + ((u32)reg * 2);
    if (address & 0xffff0001U)
        return -1;

    mac_reg_write(nic, REG_GPHY_OCP, address << 15);
    if (wait_access(nic, REG_GPHY_OCP, true))
        return -1;

    *value = (u16)(mac_reg_read(nic, REG_GPHY_OCP) & 0xffff);
    return 0;
}

int mac_write(struct my_nic *nic, u16 reg, u16 value)
{
    u32 address;

    if (reg == 0x1f) {
        nic->ocp_base = (u32)value << 4;
        return 0;
    }

    address = nic->ocp_base + reg;
    if (address & 0xffff0001U)
        return -1;

    mac_reg_write(nic, REG_MAC_OCP,
                  ACCESS_BUSY | (address << 15) | value);
    return 0;
}

int mac_read(struct my_nic *nic, u16 reg, u16 *value)
{
    u32 address;

    if (!value)
        return -1;

    address = nic->ocp_base + reg;
    if (address & 0xffff0001U)
        return -1;

    mac_reg_write(nic, REG_MAC_OCP, address << 15);
    *value = (u16)(mac_reg_read(nic, REG_MAC_OCP) & 0xffff);
    return 0;
}

static int check_device(struct my_nic *nic)
{
    nic->tx_config = mac_reg_read(nic, REG_TX_CONFIG);
    nic->xid = (nic->tx_config >> 20) & 0x0fcf;

    dev_info(&nic->pdev->dev,
             "Stage 7: TxConfig = 0x%08x, ID = 0x%03x\n",
             nic->tx_config, nic->xid);

    return nic->xid == DEVICE_XID ? 0 : -1;
}

static int reset_device(struct my_nic *nic)
{
    u8 value;
    int i;

    value = readb(nic->mmio + REG_COMMAND);
    writeb(value | RESET_BIT, nic->mmio + REG_COMMAND);

    for (i = 0; i < 100; i++) {
        if (!(readb(nic->mmio + REG_COMMAND) & RESET_BIT))
            return 0;
        udelay(10);
    }
    return -1;
}

static int read_mac(struct my_nic *nic)
{
    u32 low = mac_reg_read(nic, REG_MAC0);
    u32 high = mac_reg_read(nic, REG_MAC4);

    nic->mac[0] = low & 0xff;
    nic->mac[1] = (low >> 8) & 0xff;
    nic->mac[2] = (low >> 16) & 0xff;
    nic->mac[3] = (low >> 24) & 0xff;
    nic->mac[4] = high & 0xff;
    nic->mac[5] = (high >> 8) & 0xff;

    return is_valid_ether_addr(nic->mac) ? 0 : -1;
}

static int init_hardware(struct my_nic *nic)
{
    const struct firmware *fw = NULL;
    char version[FW_VERSION_SIZE] = { 0 };
    u32 actions = 0;
    int ret;

    if (check_device(nic))
        return -1;

    dev_info(&nic->pdev->dev, "Stage 7: resetting device\n");
    if (reset_device(nic))
        return -1;
    nic->ocp_base = OCP_STD_PHY_BASE;

    if (read_mac(nic))
        return -1;

    ret = firmware_load(nic, FW_NAME, &fw, version,
                        sizeof(version), &actions);
    if (ret)
        return -1;

    dev_info(&nic->pdev->dev,
             "Stage 7: firmware %s, actions=%u\n", version, actions);

    ret = firmware_execute(nic, fw, version, sizeof(version), &actions);
    if (!ret)
        ret = firmware_finish(nic);

    release_firmware(fw);
    return ret;
}

static int my_pci_probe(struct pci_dev *pdev,
                        const struct pci_device_id *id)
{
    struct my_nic *nic;
    int ret;

    dev_info(&pdev->dev, "Stage 7: probe started\n");

    nic = devm_kzalloc(&pdev->dev, sizeof(*nic), GFP_KERNEL);
    if (!nic)
        return -ENOMEM;

    nic->pdev = pdev;
    nic->ocp_base = OCP_STD_PHY_BASE;
    pci_set_drvdata(pdev, nic);

    ret = pci_enable_device(pdev);
    if (ret)
        return ret;

    ret = pci_request_region(pdev, 2, "my_pci_stage7");
    if (ret)
        goto disable;

    nic->mmio = pci_iomap(pdev, 2, 0);
    if (!nic->mmio) {
        ret = -ENOMEM;
        goto release;
    }

    dev_info(&pdev->dev, "Stage 7: BAR2 mapped\n");

    ret = init_hardware(nic);
    if (ret) {
        dev_err(&pdev->dev, "Stage 7: hardware initialization failed\n");
        goto unmap;
    }

    ret = netdev_create(nic);
    if (ret) {
        dev_err(&pdev->dev, "Stage 7: network device registration failed\n");
        goto unmap;
    }

    dev_info(&pdev->dev,
             "Stage 7: network stack registration completed\n");
    return 0;

unmap:
    pci_iounmap(pdev, nic->mmio);
release:
    pci_release_region(pdev, 2);
disable:
    pci_disable_device(pdev);
    return ret;
}

static void my_pci_remove(struct pci_dev *pdev)
{
    struct my_nic *nic = pci_get_drvdata(pdev);

    dev_info(&pdev->dev, "Stage 7: remove started\n");
    netdev_destroy(nic);

    if (nic && nic->mmio)
        pci_iounmap(pdev, nic->mmio);
    pci_release_region(pdev, 2);
    pci_disable_device(pdev);
    dev_info(&pdev->dev, "Stage 7: remove complete\n");
}

static struct pci_driver my_pci_driver = {
    .name = "my_pci_stage7",
    .id_table = device_ids,
    .probe = my_pci_probe,
    .remove = my_pci_remove,
};

module_pci_driver(my_pci_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("P3 Project");
MODULE_DESCRIPTION("P3 custom PCIe Ethernet Stage 7 network stack registration");
MODULE_FIRMWARE(FW_NAME);
