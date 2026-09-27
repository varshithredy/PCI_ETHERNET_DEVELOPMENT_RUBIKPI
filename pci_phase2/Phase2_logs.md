tu@ubuntu:~/pci_phase2$ vi my_pci_driver.c
ubuntu@ubuntu:~/pci_phase2$ vi my_pci_driver.h
ubuntu@ubuntu:~/pci_phase2$ vi my_pci_driver.h
ubuntu@ubuntu:~/pci_phase2$ vi Makefile
ubuntu@ubuntu:~/pci_phase2$ make
make -C /lib/modules/6.8.0-1080-qcom/build M=/home/ubuntu/pci_phase2 modules
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1080-qcom'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  CC [M]  /home/ubuntu/pci_phase2/my_pci_driver.o
  LD [M]  /home/ubuntu/pci_phase2/my_pci_stage2.o
  MODPOST /home/ubuntu/pci_phase2/Module.symvers
  CC [M]  /home/ubuntu/pci_phase2/my_pci_stage2.mod.o
  LD [M]  /home/ubuntu/pci_phase2/my_pci_stage2.ko
  BTF [M] /home/ubuntu/pci_phase2/my_pci_stage2.ko
Skipping BTF generation for /home/ubuntu/pci_phase2/my_pci_stage2.ko due to unavailability of vmlinux
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1080-qcom'
ubuntu@ubuntu:~/pci_phase2$ lspci -nnk
0000:00:00.0 PCI bridge [0604]: Qualcomm Technologies, Inc SM8250 PCIe Root Complex [Snapdragon 865/870 5G] [17cb:010b]
	Kernel driver in use: pcieport
0000:01:00.0 USB controller [0c03]: Renesas Technology Corp. uPD720201 USB 3.0 Host Controller [1912:0014] (rev 03)
	Kernel driver in use: xhci_hcd
	Kernel modules: xhci_pci
0001:00:00.0 PCI bridge [0604]: Qualcomm Technologies, Inc SM8250 PCIe Root Complex [Snapdragon 865/870 5G] [17cb:010b]
	Kernel driver in use: pcieport
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: r8169
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase2$ echo 0001:01:00.0 | sudo tee /sys/bus/pci/drivers/r8169/unbind
0001:01:00.0
ubuntu@ubuntu:~/pci_phase2$ sudo insmod ./my_pci_stage2.ko 
ubuntu@ubuntu:~/pci_phase2$ lspci -nnk
0000:00:00.0 PCI bridge [0604]: Qualcomm Technologies, Inc SM8250 PCIe Root Complex [Snapdragon 865/870 5G] [17cb:010b]
	Kernel driver in use: pcieport
0000:01:00.0 USB controller [0c03]: Renesas Technology Corp. uPD720201 USB 3.0 Host Controller [1912:0014] (rev 03)
	Kernel driver in use: xhci_hcd
	Kernel modules: xhci_pci
0001:00:00.0 PCI bridge [0604]: Qualcomm Technologies, Inc SM8250 PCIe Root Complex [Snapdragon 865/870 5G] [17cb:010b]
	Kernel driver in use: pcieport
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: my_pci_stage2
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase2$ sudo dmesg | tail -30
[   29.448860] qnoc-sc7280 1500000.interconnect: Timed out. Forcing sync_state()
[   29.448878] qnoc-sc7280 1580000.interconnect: Timed out. Forcing sync_state()
[   29.448884] qnoc-sc7280 1740000.interconnect: Timed out. Forcing sync_state()
[   29.448921] qnoc-sc7280 9100000.interconnect: Timed out. Forcing sync_state()
[   36.103708] refgen: disabling
[   84.344567] fbcon: Taking over console
[  106.414730] systemd-journald[692]: /var/log/journal/ff426f8f1c114d7061addb6c48e79742/user-1000.journal: Journal file uses a different sequence number ID, rotating.
[  106.795249] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  106.795272] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  106.795280] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  106.795287] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  106.797464] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  106.797486] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  106.797493] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  106.797500] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  106.810073] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  106.810101] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  106.810109] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  106.810117] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  106.811846] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  106.811879] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  106.811887] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  106.811894] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  470.648696] r8169 0001:01:00.0 enP1p1s0: Link is Down
[  479.296109] my_pci_stage2 0001:01:00.0: Stage 2: PCI probe started
[  479.296204] my_pci_stage2 0001:01:00.0: Stage 2: PCI ID 10ec:8161, subsystem 10ec:8168, revision 15
[  479.296210] my_pci_stage2 0001:01:00.0: Stage 2: MMIO BAR2 start=0x0000000040304000 size=0x0000000000001000
[  479.296221] my_pci_stage2 0001:01:00.0: Stage 2: TxConfig=0x57100f80, XID=0x541
[  479.296225] my_pci_stage2 0001:01:00.0: Stage 2: RTL8168H confirmed, XID=0x541
[  479.296227] my_pci_stage2 0001:01:00.0: Stage 2: probe completed successfully
ubuntu@ubuntu:~/pci_phase2$ 


