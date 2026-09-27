# PCI Ethernet Driver Development on Rubik Pi 3

Step-by-step modular development of a PCIe Realtek Gigabit Ethernet NIC driver on Qualcomm QCS6490 (Ubuntu 24.04, Linux Kernel 6.8).

## Stages
- `pci_phase1`: PCI device probe, vendor/device ID matching
- `pci_phase2`: PCI resource allocation, MMIO mapping (BAR0)
- `pci_phase3`: Hardware reset & register access checks
- `pci_phase4`: Station MAC address extraction (`eth_hw_addr_set`)
- `pci_phase5`: Hardware interrupt handling & MSI setup
- `pci_phase6`: Firmware loading & chip config initialization
- `pci_phase7`: `net_device` registration & netdev operations
- `pci_phase8`: DMA ring buffer allocation & RX/TX descriptor init
- `pci_phase9`: Full packet TX/RX datapath and NAPI polling
