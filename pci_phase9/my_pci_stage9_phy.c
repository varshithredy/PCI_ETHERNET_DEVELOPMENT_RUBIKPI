#include "my_pci_stage9.h"

#define PHY_PAGE_DEFAULT      0x0000
#define PHY_REG_BMCR          0x00
#define PHY_REG_BMSR          0x01
#define PHY_REG_ANAR          0x04
#define PHY_REG_GBCR          0x09

#define BMCR_RESET             0x8000
#define BMCR_AN_ENABLE         0x1000
#define BMCR_POWER_DOWN        0x0800
#define BMCR_ISOLATE           0x0400
#define BMCR_RESTART_ANEG      0x0200

#define BMSR_LINK              0x0004

static int page_set(struct my_nic *nic, u16 page)
{
    return phy_write(nic, 0x1f, page);
}

static int page_modify(struct my_nic *nic, u16 page, u16 reg,
                       u16 clear, u16 set)
{
    u16 value;

    if (page_set(nic, page))
        return -1;
    if (phy_read(nic, reg, &value))
        return -1;
    value &= ~clear;
    value |= set;
    return phy_write(nic, reg, value);
}

static int phy_tune(struct my_nic *nic)
{
    u16 value;
    u16 ioffset;
    u16 rlen;
    u32 data1;
    u32 data2;
    u32 data;

    /* The following are hardware parameters for the RTL8168H revision
     * used by this board. They are expressed through our generic PHY/OCP
     * access helpers rather than the reference driver's implementation. */
    if (page_set(nic, 0x0a43)) {
        dev_err(&nic->pdev->dev, "Stage 9: PHY tune step 1 page_set failed\n");
        return -1;
    }
    if (phy_write(nic, 0x13, 0x808a)) {
        dev_err(&nic->pdev->dev, "Stage 9: PHY tune step 1 phy_write failed\n");
        return -1;
    }
    if (phy_read(nic, 0x14, &value)) {
        dev_err(&nic->pdev->dev, "Stage 9: PHY tune step 1 phy_read failed\n");
        return -1;
    }
    value = (value & ~0x003f) | 0x000a;
    if (phy_write(nic, 0x14, value))
        return -1;

    if (page_set(nic, 0x0a43) ||
        phy_write(nic, 0x13, 0x0811) ||
        phy_read(nic, 0x14, &value))
        return -1;
    value |= 0x0800;
    if (phy_write(nic, 0x14, value))
        return -1;

    if (page_modify(nic, 0x0a42, 0x16, 0x0000, 0x0002))
        return -1;
    if (page_modify(nic, 0x0a44, 0x11, 0x0000, 0x0800))
        return -1;

    if (mac_write(nic, 0xdd02, 0x807d))
        return -1;
    if (mac_read(nic, 0xdd02, &ioffset))
        return -1;
    if (mac_read(nic, 0xdd00, &value))
        return -1;

    data1 = ioffset;
    data2 = value;
    data = ((data2 >> 1) & 0x7ff8) | (data2 & 0x0007);
    if (data1 & BIT(7))
        data |= BIT(15);

    if (data != 0xffff) {
        if (page_set(nic, 0x0bcf) ||
            phy_write(nic, 0x16, (u16)data))
            return -1;
    }

    if (page_set(nic, 0x0bcd) ||
        phy_read(nic, 0x16, &value))
        return -1;
    rlen = (value & 0x000f);
    if (rlen > 3)
        rlen -= 3;
    else
        rlen = 0;
    data = rlen | ((u32)rlen << 4) | ((u32)rlen << 8) |
           ((u32)rlen << 12);
    if (phy_write(nic, 0x17, (u16)data))
        return -1;

    if (page_modify(nic, 0x0a44, 0x11, 0x0080, 0x0000))
        return -1;
    if (page_modify(nic, 0x0a43, 0x10, 0x0001, 0x0000))
        return -1;
    if (page_modify(nic, 0x0a43, 0x10, 0x0004, 0x0000))
        return -1;
    if (page_modify(nic, 0x0a43, 0x11, 0x0000, 0x0010))
        return -1;

    if (page_set(nic, PHY_PAGE_DEFAULT))
        return -1;

    return 0;
}

static void phy_debug(struct my_nic *nic)
{
    u16 bmcr = 0;
    u16 bmsr = 0;
    u16 anar = 0;
    u16 gbcr = 0;
    u8 status;

    phy_read(nic, PHY_REG_BMCR, &bmcr);
    phy_read(nic, PHY_REG_BMSR, &bmsr);
    phy_read(nic, PHY_REG_ANAR, &anar);
    phy_read(nic, PHY_REG_GBCR, &gbcr);

    status = readb(nic->mmio + REG_PHY_STATUS);

    dev_info(&nic->pdev->dev,
             "Stage 9 PHY: BMCR=0x%04x BMSR=0x%04x ANAR=0x%04x GBCR=0x%04x STATUS=0x%02x\\n",
             bmcr, bmsr, anar, gbcr, status);
}

int phy_link_start(struct my_nic *nic)
{
    u16 bmcr;
    u16 bmsr = 0;
    int i;

    dev_info(&nic->pdev->dev,
             "Stage 9: starting PHY configuration and auto-negotiation\n");

    if (phy_tune(nic)) {
        dev_err(&nic->pdev->dev,
                "Stage 9: PHY hardware configuration failed\n");
        return -1;
    }

    /*
     * The working reference driver performs a PHY soft reset after
     * the RTL8168H-specific tuning.  Keep that ordering while using
     * our own simple PHY access layer.
     */
    if (phy_read(nic, PHY_REG_BMCR, &bmcr)) {
        dev_err(&nic->pdev->dev,
                "Stage 9: PHY BMCR read failed before reset\n");
        return -1;
    }

    bmcr |= BMCR_RESET;

    if (phy_write(nic, PHY_REG_BMCR, bmcr)) {
        dev_err(&nic->pdev->dev,
                "Stage 9: PHY reset write failed\n");
        return -1;
    }

    for (i = 0; i < 100; i++) {
        if (phy_read(nic, PHY_REG_BMCR, &bmcr)) {
            dev_err(&nic->pdev->dev,
                    "Stage 9: PHY BMCR read failed during reset\n");
            return -1;
        }

        if (!(bmcr & BMCR_RESET))
            break;

        msleep(1);
    }

    if (bmcr & BMCR_RESET) {
        dev_err(&nic->pdev->dev,
                "Stage 9: PHY reset did not complete, BMCR=0x%04x\n",
                bmcr);
        return -1;
    }

    bmcr &= ~(BMCR_POWER_DOWN | BMCR_ISOLATE);
    bmcr |= BMCR_AN_ENABLE | BMCR_RESTART_ANEG;

    if (phy_write(nic, PHY_REG_BMCR, bmcr)) {
        dev_err(&nic->pdev->dev,
                "Stage 9: PHY auto-negotiation start failed\n");
        return -1;
    }

    dev_info(&nic->pdev->dev,
             "Stage 9: PHY BMCR=0x%04x, auto-negotiation started\n", bmcr);

    phy_debug(nic);

    for (i = 0; i < 300; i++) {
        if (phy_read(nic, PHY_REG_BMSR, &bmsr)) {
            dev_err(&nic->pdev->dev,
                    "Stage 9: PHY BMSR read failed while waiting for link\n");
            return -1;
        }

        /* Read twice because the link bit can be latch-low. */
        if (bmsr & BMSR_LINK) {
            if (phy_read(nic, PHY_REG_BMSR, &bmsr)) {
                dev_err(&nic->pdev->dev,
                        "Stage 9: PHY BMSR second read failed\n");
                return -1;
            }

            if (bmsr & BMSR_LINK) {
                dev_info(&nic->pdev->dev,
                         "Stage 9: PHY link detected, BMSR=0x%04x\n",
                         bmsr);
                return 0;
            }
        }

        msleep(10);
    }

    dev_info(&nic->pdev->dev,
             "Stage 9: PHY link not detected yet, BMSR=0x%04x\n", bmsr);

    return 0;
}
