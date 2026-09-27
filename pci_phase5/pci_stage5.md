ubuntu@ubuntu:~/pci_phase5$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: r8169
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase5$ echo 0001:01:00.0 | sudo tee /sys/bus/pci/drivers/r8169/unbind
0001:01:00.0
ubuntu@ubuntu:~/pci_phase5$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase5$ sudo insmod ./my_pci_stage5.ko 
ubuntu@ubuntu:~/pci_phase5$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: my_pci_stage5
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase5$ sudo dmesg | grep "stage 5"
ubuntu@ubuntu:~/pci_phase5$ sudo dmesg | grep "stage 5"
ubuntu@ubuntu:~/pci_phase5$ sudo dmesg | grep "Stage 5"
[  482.452425] my_pci_stage5 0001:01:00.0: Stage 5: probe started
[  482.452513] my_pci_stage5 0001:01:00.0: Stage 5: BAR2 mapped
[  482.452523] my_pci_stage5 0001:01:00.0: Stage 5: TxConfig = 0x57100f80, ID = 0x541
[  482.452553] my_pci_stage5 0001:01:00.0: Stage 5: device check passed
[  482.452555] my_pci_stage5 0001:01:00.0: Stage 5: resetting device
[  482.452573] my_pci_stage5 0001:01:00.0: Stage 5: reset complete
[  482.452620] my_pci_stage5 0001:01:00.0: Stage 5: MAC address 58:04:4f:67:5d:69
[  482.452623] my_pci_stage5 0001:01:00.0: Stage 5: asking for firmware rtl_nic/rtl8168h-2.fw
[  482.452779] my_pci_stage5 0001:01:00.0: Stage 5: firmware found, size = 976 bytes
[  482.452784] my_pci_stage5 0001:01:00.0: Stage 5: firmware is ready in memory
ubuntu@ubuntu:~/pci_phase5$ ls -lh /lib/firmware/rtl_nic/rtl8168h-2.fw
ls: cannot access '/lib/firmware/rtl_nic/rtl8168h-2.fw': No such file or directory
ubuntu@ubuntu:~/pci_phase5$ sudo ls -lh /lib/firmware/rtl_nic/rtl8168h-2.fw
ls: cannot access '/lib/firmware/rtl_nic/rtl8168h-2.fw': No such file or directory
ubuntu@ubuntu:~/pci_phase5$ sudo rmmod my_pci_stage5 
ubuntu@ubuntu:~/pci_phase5$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase5$ echo 0001:01:00.0 | sudo tee /sys/bus/pci/drivers/r8169/bind 
0001:01:00.0
ubuntu@ubuntu:~/pci_phase5$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: r8169
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase5$ 

