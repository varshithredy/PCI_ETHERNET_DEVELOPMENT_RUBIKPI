#ifndef MY_PCI_STAGE6_H
#define MY_PCI_STAGE6_H

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/errno.h>

#define DEVICE_VENDOR_ID      0x10ec
#define DEVICE_ID             0x8161
#define DEVICE_SUBVENDOR      0x10ec
#define DEVICE_SUBDEVICE      0x8168
#define DEVICE_XID            0x541

#define REG_MAC0              0x00
#define REG_MAC4              0x04
#define REG_COMMAND           0x37
#define REG_TX_CONFIG         0x40
#define REG_GPHY_OCP          0xB8
#define REG_MAC_OCP           0xB0

#define RESET_BIT             0x10
#define ACCESS_BUSY           0x80000000U
#define OCP_STD_PHY_BASE     0xA400
#define FW_NAME               "rtl_nic/rtl8168h-2.fw"
#define FW_VERSION_SIZE       32
#define FW_MAX_ACTIONS        4096
#define PHY_RESET_BIT         0x8000

struct my_nic {
    struct pci_dev *pdev;
    void __iomem *mmio;
    u16 xid;
    u32 tx_config;
    u32 ocp_base;
    u8 mac[ETH_ALEN];
};

u32 mac_reg_read(struct my_nic *nic, u32 reg);
void mac_reg_write(struct my_nic *nic, u32 reg, u32 value);

int phy_read(struct my_nic *nic, u16 reg, u16 *value);
int phy_write(struct my_nic *nic, u16 reg, u16 value);
int mac_read(struct my_nic *nic, u16 reg, u16 *value);
int mac_write(struct my_nic *nic, u16 reg, u16 value);

int firmware_load(struct my_nic *nic, const char *name,
                  const struct firmware **fw, char *version,
                  size_t version_size, u32 *actions);
int firmware_execute(struct my_nic *nic, const struct firmware *fw,
                     char *version, size_t version_size, u32 *actions);
int firmware_finish(struct my_nic *nic);

#endif
