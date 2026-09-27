#ifndef MY_PCI_STAGE9_H
#define MY_PCI_STAGE9_H

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/timer.h>
#include <linux/errno.h>

#define DEVICE_VENDOR_ID      0x10ec
#define DEVICE_ID             0x8161
#define DEVICE_SUBVENDOR      0x10ec
#define DEVICE_SUBDEVICE      0x8168
#define DEVICE_XID            0x541

#define REG_MAC0              0x00
#define REG_MAC4              0x04
#define REG_COMMAND           0x37
#define REG_TX_POLL           0x38
#define REG_INTR_MASK         0x3c
#define REG_INTR_STATUS       0x3e
#define REG_TX_CONFIG         0x40
#define REG_RX_CONFIG         0x44
#define REG_GPHY_OCP          0xB8
#define REG_MAC_OCP           0xB0
#define REG_RX_MAX_SIZE       0xDA
#define REG_TX_RING_LOW       0x20
#define REG_TX_RING_HIGH      0x24
#define REG_RX_RING_LOW       0xE4
#define REG_RX_RING_HIGH      0xE8
#define REG_PHY_STATUS        0x6C

#define RESET_BIT             0x10
#define RX_ENABLE             0x08
#define TX_ENABLE             0x04
#define TX_POLL_NORMAL        0x40
#define ACCESS_BUSY           0x80000000U
#define OCP_STD_PHY_BASE      0xA400
#define FW_NAME               "rtl_nic/rtl8168h-2.fw"
#define FW_VERSION_SIZE       32
#define FW_MAX_ACTIONS        4096
#define PHY_RESET_BIT         0x8000

#define RING_COUNT             64
#define RX_BUF_SIZE            16383
#define RX_STATUS_OWN          BIT(31)
#define RX_STATUS_EOR          BIT(30)
#define TX_STATUS_FIRST        BIT(29)
#define TX_STATUS_LAST         BIT(28)
#define TX_STATUS_OWN          BIT(31)
#define TX_STATUS_EOR          BIT(30)
#define DESC_LENGTH_MASK       0x3fff
#define PHY_LINK_BIT           BIT(1)

#define RXCFG_ACCEPT_BROADCAST BIT(3)
#define RXCFG_ACCEPT_MULTICAST BIT(2)
#define RXCFG_ACCEPT_PHYS      BIT(1)
#define RXCFG_DMA_BURST        (7U << 8)
#define RXCFG_EARLY_OFF        BIT(11)
#define RXCFG_128_INT          BIT(15)
#define RXCFG_MULTIPLE         BIT(14)

struct dma_desc {
    __le32 status;
    __le32 control;
    __le64 address;
};

struct tx_slot {
    struct sk_buff *skb;
    dma_addr_t dma;
    u32 length;
};

struct rx_slot {
    struct sk_buff *skb;
    dma_addr_t dma;
};

struct my_nic {
    struct pci_dev *pdev;
    struct net_device *netdev;
    void __iomem *mmio;
    u16 xid;
    u32 tx_config;
    u32 ocp_base;
    u8 mac[ETH_ALEN];

    struct dma_desc *tx_ring;
    struct dma_desc *rx_ring;
    dma_addr_t tx_ring_dma;
    dma_addr_t rx_ring_dma;
    struct tx_slot tx_slot[RING_COUNT];
    struct rx_slot rx_slot[RING_COUNT];
    u16 tx_head;
    u16 tx_tail;
    u16 rx_head;
    spinlock_t tx_lock;
    struct timer_list poll_timer;
    bool dma_running;
    bool tx_reported;
    bool tx_reclaim_reported;
    bool rx_reported;
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
int phy_link_start(struct my_nic *nic);

int dma_setup(struct my_nic *nic);
void dma_cleanup(struct my_nic *nic);
int dma_start(struct my_nic *nic);
void dma_stop(struct my_nic *nic);
netdev_tx_t dma_transmit(struct sk_buff *skb, struct net_device *dev);
void dma_poll(struct timer_list *timer);

int netdev_create(struct my_nic *nic);
void netdev_destroy(struct my_nic *nic);

#endif
