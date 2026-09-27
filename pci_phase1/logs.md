ubuntu@ubuntu:~/pci_phase1$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: r8169
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase1$ make
make -C /lib/modules/6.8.0-1080-qcom/build M=/home/ubuntu/pci_phase1 modules
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1080-qcom'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1080-qcom'
ubuntu@ubuntu:~/pci_phase1$ echo 0001:01:00.0 | sudo tee /sys/bus/pci/drivers/r8169/unbind
0001:01:00.0
ubuntu@ubuntu:~/pci_phase1$ make
make -C /lib/modules/6.8.0-1080-qcom/build M=/home/ubuntu/pci_phase1 modules
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1080-qcom'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  CC [M]  /home/ubuntu/pci_phase1/my_pci_driver.o
  LD [M]  /home/ubuntu/pci_phase1/my_pci_stage1.o
  MODPOST /home/ubuntu/pci_phase1/Module.symvers
  CC [M]  /home/ubuntu/pci_phase1/my_pci_stage1.mod.o
  LD [M]  /home/ubuntu/pci_phase1/my_pci_stage1.ko
  BTF [M] /home/ubuntu/pci_phase1/my_pci_stage1.ko
Skipping BTF generation for /home/ubuntu/pci_phase1/my_pci_stage1.ko due to unavailability of vmlinux
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1080-qcom'
ubuntu@ubuntu:~/pci_phase1$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase1$ sudo insmod ./my_pci_stage1.ko 
ubuntu@ubuntu:~/pci_phase1$ lspci -nnk -s 0001:01:00.0
sudo dmesg | tail -30
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: my_pci_stage1
	Kernel modules: r8169
[   27.313121] r8169 0001:01:00.0 enP1p1s0: Link is Up - 1Gbps/Full - flow control rx/tx
[   28.935286] qnoc-sc7280 1500000.interconnect: Timed out. Forcing sync_state()
[   28.935321] qnoc-sc7280 1580000.interconnect: Timed out. Forcing sync_state()
[   28.935336] qnoc-sc7280 1740000.interconnect: Timed out. Forcing sync_state()
[   28.935423] qnoc-sc7280 9100000.interconnect: Timed out. Forcing sync_state()
[   36.102259] refgen: disabling
[   83.501967] fbcon: Taking over console
[  171.671886] systemd-journald[677]: /var/log/journal/ff426f8f1c114d7061addb6c48e79742/user-1000.journal: Journal file uses a different sequence number ID, rotating.
[  171.970272] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  171.970295] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  171.970303] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  171.970310] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  171.970822] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  171.970838] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  171.970845] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  171.970852] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  171.974634] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  171.974659] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  171.974666] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  171.974673] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  171.975142] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  171.975167] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  171.975174] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  171.975181] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  197.848728] r8169 0001:01:00.0 enP1p1s0: Link is Down
[  640.281345] my_pci_stage1 0001:01:00.0: Stage 1: PCI probe started
[  640.281458] my_pci_stage1 0001:01:00.0: Stage 1: RTL8168H PCI device detected
[  640.281462] my_pci_stage1 0001:01:00.0: PCI ID 10ec:8161, subsystem 10ec:8168, revision 15
[  640.281466] my_pci_stage1 0001:01:00.0: MMIO BAR2: start=0x0000000040304000 size=0x0000000000001000 mapped=000000008070391c
[  640.281471] my_pci_stage1 0001:01:00.0: Stage 1: probe completed successfully
ubuntu@ubuntu:~/pci_phase1$ sudo rmmod my_pci_stage1 
