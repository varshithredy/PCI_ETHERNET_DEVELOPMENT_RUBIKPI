#include "my_pci_stage9.h"

static void tx_reclaim(struct my_nic *nic)
{
    while (nic->tx_tail != nic->tx_head) {
        struct dma_desc *d = &nic->tx_ring[nic->tx_tail];
        u32 status = le32_to_cpu(READ_ONCE(d->status));
        struct tx_slot *slot = &nic->tx_slot[nic->tx_tail];

        if (status & TX_STATUS_OWN)
            break;

        if (slot->skb) {
            dma_unmap_single(&nic->pdev->dev, slot->dma,
                             slot->length, DMA_TO_DEVICE);
            dev_consume_skb_any(slot->skb);
            slot->skb = NULL;

            if (!nic->tx_reclaim_reported) {
                dev_info(&nic->pdev->dev,
                         "Stage 9: first TX descriptor completed by hardware\n");
                nic->tx_reclaim_reported = true;
            }
        }

        WRITE_ONCE(d->address, cpu_to_le64(0));
        WRITE_ONCE(d->status, cpu_to_le32(0));
        nic->tx_tail = (nic->tx_tail + 1) % RING_COUNT;
        netif_wake_queue(nic->netdev);
    }
}

static int rx_refill_one(struct my_nic *nic, u16 index)
{
    struct rx_slot *slot = &nic->rx_slot[index];
    struct dma_desc *d = &nic->rx_ring[index];
    struct sk_buff *skb;
    dma_addr_t dma;
    u32 status = RX_BUF_SIZE;

    skb = netdev_alloc_skb_ip_align(nic->netdev, RX_BUF_SIZE);
    if (!skb)
        return -1;

    dma = dma_map_single(&nic->pdev->dev, skb->data, RX_BUF_SIZE,
                         DMA_FROM_DEVICE);
    if (dma_mapping_error(&nic->pdev->dev, dma)) {
        dev_kfree_skb_any(skb);
        return -1;
    }

    slot->skb = skb;
    slot->dma = dma;
    d->address = cpu_to_le64(dma);
    if (index == RING_COUNT - 1)
        status |= RX_STATUS_EOR;

    dma_wmb();
    WRITE_ONCE(d->status,cpu_to_le32(status|RX_STATUS_OWN));
    return 0;
}

static int rx_setup(struct my_nic *nic)
{
    u16 i;

    for (i = 0; i < RING_COUNT; i++) {
        if (rx_refill_one(nic, i)) {
            while (i--)
                if (nic->rx_slot[i].skb) {
                    dma_unmap_single(&nic->pdev->dev, nic->rx_slot[i].dma,
                                     RX_BUF_SIZE, DMA_FROM_DEVICE);
                    dev_kfree_skb_any(nic->rx_slot[i].skb);
                    nic->rx_slot[i].skb = NULL;
                }
            return -1;
        }
    }
    nic->rx_head = 0;
    return 0;
}

static void rx_cleanup(struct my_nic *nic)
{
    u16 i;

    for (i = 0; i < RING_COUNT; i++) {
        if (!nic->rx_slot[i].skb)
            continue;
        dma_unmap_single(&nic->pdev->dev, nic->rx_slot[i].dma,
                         RX_BUF_SIZE, DMA_FROM_DEVICE);
        dev_kfree_skb_any(nic->rx_slot[i].skb);
        nic->rx_slot[i].skb = NULL;
        nic->rx_slot[i].dma = 0;
    }
}

static bool rx_debug_reported;

static void rx_debug(struct my_nic *nic)
{
    struct dma_desc *d = &nic->rx_ring[nic->rx_head];
    u32 status = le32_to_cpu(READ_ONCE(d->status));

    if (rx_debug_reported)
        return;

    dev_info(&nic->pdev->dev,
             "Stage 9 RX: index=%u status=0x%08x address=%pad\\n",
             nic->rx_head, status, &d->address);

    dev_info(&nic->pdev->dev,
             "Stage 9 RX: RXCFG=0x%08x RXMAX=%u\\n",
             readl(nic->mmio + REG_RX_CONFIG),
             readw(nic->mmio + REG_RX_MAX_SIZE));

    dev_info(&nic->pdev->dev,
             "Stage 9 RX: ring_low=0x%08x ring_high=0x%08x\\n",
             readl(nic->mmio + REG_RX_RING_LOW),
             readl(nic->mmio + REG_RX_RING_HIGH));

    rx_debug_reported = true;
}

static void rx_process(struct my_nic *nic)
{
    int budget = 32;

    rx_debug(nic);

    while (budget--) {
        u16 index = nic->rx_head;
        struct dma_desc *d = &nic->rx_ring[index];
        struct rx_slot *slot = &nic->rx_slot[index];
        u32 status = le32_to_cpu(READ_ONCE(d->status));
        u32 length;
        struct sk_buff *skb;
        dma_addr_t old_dma;

        if (status & RX_STATUS_OWN)
            break;

        skb = slot->skb;
        old_dma = slot->dma;
        if (!skb)
            break;

        length = status & DESC_LENGTH_MASK;

        /* Hardware includes the Ethernet FCS in the received length. */
        if (length >= ETH_FCS_LEN)
            length -= ETH_FCS_LEN;

        dma_rmb();

        dma_unmap_single(&nic->pdev->dev, old_dma, RX_BUF_SIZE,
                         DMA_FROM_DEVICE);
        slot->skb = NULL;
        slot->dma = 0;

        if (length >= ETH_HLEN && length <= RX_BUF_SIZE - ETH_FCS_LEN) {
            skb_put(skb, length);
            skb->protocol = eth_type_trans(skb, nic->netdev);
            netif_rx(skb);
            nic->netdev->stats.rx_packets++;
            nic->netdev->stats.rx_bytes += length;

            if (!nic->rx_reported) {
                dev_info(&nic->pdev->dev,
                         "Stage 9: first RX packet received, length=%u\n",
                         length);
                nic->rx_reported = true;
            }
        } else {
            dev_kfree_skb_any(skb);
            nic->netdev->stats.rx_errors++;
        }

        if (rx_refill_one(nic, index)) {
            dev_err(&nic->pdev->dev,
                    "Stage 9: RX buffer refill failed at descriptor %u\n",
                    index);
            break;
        }

        nic->rx_head = (nic->rx_head + 1) % RING_COUNT;
    }
}

int dma_setup(struct my_nic *nic)
{
    size_t bytes = sizeof(struct dma_desc) * RING_COUNT;

    nic->tx_ring = dma_alloc_coherent(&nic->pdev->dev, bytes,
                                      &nic->tx_ring_dma, GFP_KERNEL);
    if (!nic->tx_ring)
        return -1;

    nic->rx_ring = dma_alloc_coherent(&nic->pdev->dev, bytes,
                                      &nic->rx_ring_dma, GFP_KERNEL);
    if (!nic->rx_ring) {
        dma_free_coherent(&nic->pdev->dev, bytes,
                          nic->tx_ring, nic->tx_ring_dma);
        nic->tx_ring = NULL;
        return -1;
    }

    memset(nic->tx_ring, 0, bytes);
    memset(nic->rx_ring, 0, bytes);
    memset(nic->tx_slot, 0, sizeof(nic->tx_slot));
    memset(nic->rx_slot, 0, sizeof(nic->rx_slot));
    spin_lock_init(&nic->tx_lock);
    nic->tx_head = 0;
    nic->tx_tail = 0;
    nic->rx_head = 0;

    if (rx_setup(nic)) {
        dma_cleanup(nic);
        return -1;
    }

    dev_info(&nic->pdev->dev,
             "Stage 9: DMA rings allocated TX=%zu RX=%zu descriptors\n",
             (size_t)RING_COUNT, (size_t)RING_COUNT);
    dev_info(&nic->pdev->dev,
             "Stage 9: TX DMA=%pad RX DMA=%pad\n",
             &nic->tx_ring_dma, &nic->rx_ring_dma);
    return 0;
}

void dma_cleanup(struct my_nic *nic)
{
    size_t bytes = sizeof(struct dma_desc) * RING_COUNT;
    u16 i;

    if (!nic)
        return;

    for (i = 0; i < RING_COUNT; i++) {
        if (nic->tx_slot[i].skb) {
            dma_unmap_single(&nic->pdev->dev, nic->tx_slot[i].dma,
                             nic->tx_slot[i].length, DMA_TO_DEVICE);
            dev_kfree_skb_any(nic->tx_slot[i].skb);
            nic->tx_slot[i].skb = NULL;
        }
    }

    rx_cleanup(nic);

    if (nic->rx_ring) {
        dma_free_coherent(&nic->pdev->dev, bytes,
                          nic->rx_ring, nic->rx_ring_dma);
        nic->rx_ring = NULL;
    }
    if (nic->tx_ring) {
        dma_free_coherent(&nic->pdev->dev, bytes,
                          nic->tx_ring, nic->tx_ring_dma);
        nic->tx_ring = NULL;
    }
}

static void dma_program_rings(struct my_nic *nic)
{
    writel(upper_32_bits(nic->tx_ring_dma),nic->mmio+REG_TX_RING_HIGH);
    writel(lower_32_bits(nic->tx_ring_dma),nic->mmio+REG_TX_RING_LOW);
    writel(upper_32_bits(nic->rx_ring_dma),nic->mmio+REG_RX_RING_HIGH);
    writel(lower_32_bits(nic->rx_ring_dma),nic->mmio+REG_RX_RING_LOW);
    wmb();
}

int dma_start(struct my_nic *nic)
{
    u32 rx_cfg;

    dma_program_rings(nic);

    writew(RX_BUF_SIZE+1,nic->mmio+REG_RX_MAX_SIZE);

    writeb(readb(nic->mmio+REG_COMMAND)|TX_ENABLE|RX_ENABLE,
           nic->mmio+REG_COMMAND);

    rx_cfg=RXCFG_128_INT|RXCFG_MULTIPLE|
           RXCFG_DMA_BURST|RXCFG_EARLY_OFF|
           RXCFG_ACCEPT_BROADCAST|RXCFG_ACCEPT_MULTICAST|
           RXCFG_ACCEPT_PHYS;
    writel(rx_cfg,nic->mmio+REG_RX_CONFIG);

    writel(nic->tx_config,nic->mmio+REG_TX_CONFIG);

    dev_info(&nic->pdev->dev,
             "Stage 9 DMA: CMD=0x%02x RXCFG=0x%08x RXMAX=%u TXCFG=0x%08x\\n",
             readb(nic->mmio+REG_COMMAND),
             readl(nic->mmio+REG_RX_CONFIG),
             readw(nic->mmio+REG_RX_MAX_SIZE),
             readl(nic->mmio+REG_TX_CONFIG));

    nic->dma_running=true;
    mod_timer(&nic->poll_timer,jiffies+msecs_to_jiffies(10));

    dev_info(&nic->pdev->dev,
             "Stage 9: DMA engine started\\n");
    dev_info(&nic->pdev->dev,
             "Stage 9: TX/RX packet DMA path ready\\n");
    return 0;
}

void dma_stop(struct my_nic *nic)
{
    if (!nic->dma_running)
        return;

    nic->dma_running = false;
    del_timer_sync(&nic->poll_timer);
    writeb(readb(nic->mmio + REG_COMMAND) & ~(TX_ENABLE | RX_ENABLE),
           nic->mmio + REG_COMMAND);
    netif_stop_queue(nic->netdev);
    dev_info(&nic->pdev->dev, "Stage 9: DMA engine stopped\n");
}

netdev_tx_t dma_transmit(struct sk_buff *skb, struct net_device *dev)
{
    struct my_nic *nic = netdev_priv(dev);
    unsigned long flags;
    u16 next;
    struct dma_desc *d;
    struct tx_slot *slot;
    dma_addr_t dma;
    u32 status;

    if (!nic->dma_running)
        return NETDEV_TX_BUSY;

    spin_lock_irqsave(&nic->tx_lock, flags);

    next = (nic->tx_head + 1) % RING_COUNT;
    if (next == nic->tx_tail) {
        netif_stop_queue(dev);
        spin_unlock_irqrestore(&nic->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    dma = dma_map_single(&nic->pdev->dev, skb->data, skb->len,
                         DMA_TO_DEVICE);
    if (dma_mapping_error(&nic->pdev->dev, dma)) {
        spin_unlock_irqrestore(&nic->tx_lock, flags);
        dev_kfree_skb_any(skb);
        dev->stats.tx_errors++;
        return NETDEV_TX_OK;
    }

    d = &nic->tx_ring[nic->tx_head];
    slot = &nic->tx_slot[nic->tx_head];
    slot->skb = skb;
    slot->dma = dma;
    slot->length = skb->len;

    status = skb->len & DESC_LENGTH_MASK;
    status |= TX_STATUS_FIRST | TX_STATUS_LAST;
    if (nic->tx_head == RING_COUNT - 1)
        status |= TX_STATUS_EOR;

    d->address = cpu_to_le64(dma);
    d->control = 0;

    /* Publish address/options before giving ownership to the NIC. */
    dma_wmb();
    WRITE_ONCE(d->status, cpu_to_le32(status | TX_STATUS_OWN));

    nic->tx_head = next;
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;

    if (!nic->tx_reported) {
        dev_info(&nic->pdev->dev,
                 "Stage 9: first TX packet queued, length=%u\n",
                 skb->len);
        nic->tx_reported = true;
    }

    writeb(TX_POLL_NORMAL, nic->mmio + REG_TX_POLL);

    if (((nic->tx_head + 1) % RING_COUNT) == nic->tx_tail)
        netif_stop_queue(dev);

    spin_unlock_irqrestore(&nic->tx_lock, flags);
    return NETDEV_TX_OK;
}

void dma_poll(struct timer_list *timer)
{
    struct my_nic *nic = from_timer(nic, timer, poll_timer);
    u8 phy;

    if (!nic->dma_running)
        return;

    tx_reclaim(nic);
    rx_process(nic);

    phy = readb(nic->mmio + REG_PHY_STATUS);
    if (phy & PHY_LINK_BIT)
        netif_carrier_on(nic->netdev);
    else
        netif_carrier_off(nic->netdev);

    mod_timer(&nic->poll_timer, jiffies + msecs_to_jiffies(10));
}
