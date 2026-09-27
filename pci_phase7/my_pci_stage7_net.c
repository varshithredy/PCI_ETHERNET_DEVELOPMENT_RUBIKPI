#include "my_pci_stage7.h"

static int net_open(struct net_device *dev)
{
    struct my_nic *nic = netdev_priv(dev);

    netif_start_queue(dev);
    netif_carrier_off(dev);

    dev_info(&nic->pdev->dev, "Stage 7: network interface opened\n");
    return 0;
}

static int net_stop(struct net_device *dev)
{
    struct my_nic *nic = netdev_priv(dev);

    netif_stop_queue(dev);
    netif_carrier_off(dev);

    dev_info(&nic->pdev->dev, "Stage 7: network interface stopped\n");
    return 0;
}

static netdev_tx_t net_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct my_nic *nic = netdev_priv(dev);

    dev_kfree_skb(skb);
    netif_stop_queue(dev);
    netif_wake_queue(dev);

    dev_warn_ratelimited(&nic->pdev->dev,
                         "Stage 7: TX requested before DMA engine is implemented\n");
    return NETDEV_TX_OK;
}

static int net_set_mac(struct net_device *dev, void *addr)
{
    struct sockaddr *sa = addr;

    if (!is_valid_ether_addr(sa->sa_data))
        return -EINVAL;

    eth_hw_addr_set(dev, sa->sa_data);
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
    struct net_device *dev;
    int ret;

    dev = alloc_etherdev(sizeof(*nic));
    if (!dev)
        return -1;

    memcpy(netdev_priv(dev), nic, sizeof(*nic));
    nic = netdev_priv(dev);
    nic->netdev = dev;

    dev->netdev_ops = &net_ops;
    dev->min_mtu = 60;
    dev->max_mtu = 1500;
    eth_hw_addr_set(dev, nic->mac);
    netif_carrier_off(dev);

    ret = register_netdev(dev);
    if (ret) {
        free_netdev(dev);
        return -1;
    }

    /* Keep the PCI driver's copy pointing at the live netdev-private state. */
    pci_set_drvdata(nic->pdev, nic);
    dev_info(&nic->pdev->dev,
             "Stage 7: network device registered as %s\n", dev->name);
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
