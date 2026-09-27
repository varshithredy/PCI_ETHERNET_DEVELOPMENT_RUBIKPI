#include "my_pci_stage9.h"

static int net_open(struct net_device *dev)
{
    struct my_nic *nic = netdev_priv(dev);

    /*
     * Configure the RTL8168H PHY and start auto-negotiation before
     * bringing up the MAC DMA datapath.
     */
    if (phy_link_start(nic))
        return -1;

    if (dma_start(nic))
        return -1;

    netif_start_queue(dev);
    dev_info(&nic->pdev->dev, "Stage 9: network interface opened\n");
    return 0;
}

static int net_stop(struct net_device *dev)
{
    struct my_nic *nic = netdev_priv(dev);

    netif_stop_queue(dev);
    dma_stop(nic);
    dev_info(&nic->pdev->dev, "Stage 9: network interface stopped\n");
    return 0;
}

static netdev_tx_t net_xmit(struct sk_buff *skb, struct net_device *dev)
{
    return dma_transmit(skb, dev);
}

static int net_set_mac(struct net_device *dev, void *addr)
{
    struct sockaddr *sa = addr;
    struct my_nic *nic = netdev_priv(dev);

    if (!is_valid_ether_addr(sa->sa_data))
        return -EINVAL;

    eth_hw_addr_set(dev, sa->sa_data);
    writel(get_unaligned_le32(sa->sa_data), nic->mmio + REG_MAC0);
    writew(get_unaligned_le16(sa->sa_data + 4), nic->mmio + REG_MAC4);
    return 0;
}

static const struct net_device_ops net_ops = {
    .ndo_open = net_open,
    .ndo_stop = net_stop,
    .ndo_start_xmit = net_xmit,
    .ndo_set_mac_address = net_set_mac,
    .ndo_validate_addr = eth_validate_addr,
};

int netdev_create(struct my_nic *nic)
{
    struct net_device *dev = nic->netdev;
    int ret;

    timer_setup(&nic->poll_timer, dma_poll, 0);
    dev->netdev_ops = &net_ops;
    dev->min_mtu = ETH_MIN_MTU;
    dev->max_mtu = 1500;
    eth_hw_addr_set(dev, nic->mac);
    netif_carrier_off(dev);

    ret = register_netdev(dev);
    if (ret)
        return -1;

    dev_info(&nic->pdev->dev,
             "Stage 9: network device registered as %s\n", dev->name);
    return 0;
}

void netdev_destroy(struct my_nic *nic)
{
    struct net_device *dev;

    if (!nic || !nic->netdev)
        return;

    dev = nic->netdev;
    unregister_netdev(dev);
    free_netdev(dev);
    nic->netdev = NULL;
}
