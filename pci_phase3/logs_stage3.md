ubuntu@ubuntu:~/pci_phase3$ make
make -C /lib/modules/6.8.0-1080-qcom/build M=/home/ubuntu/pci_phase3 modules
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1080-qcom'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  CC [M]  /home/ubuntu/pci_phase3/my_pci_driver.o
  LD [M]  /home/ubuntu/pci_phase3/my_pci_stage3.o
  MODPOST /home/ubuntu/pci_phase3/Module.symvers
  CC [M]  /home/ubuntu/pci_phase3/my_pci_stage3.mod.o
  LD [M]  /home/ubuntu/pci_phase3/my_pci_stage3.ko
  BTF [M] /home/ubuntu/pci_phase3/my_pci_stage3.ko
Skipping BTF generation for /home/ubuntu/pci_phase3/my_pci_stage3.ko due to unavailability of vmlinux
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1080-qcom'
ubuntu@ubuntu:~/pci_phase3$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: r8169
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase3$ echo 0001:01:00.0 | sudo tee /sys/bus/pci/drivers/r8169/unbind
0001:01:00.0
ubuntu@ubuntu:~/pci_phase3$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase3$ sudo insmod ./my_pci_stage3.ko 
ubuntu@ubuntu:~/pci_phase3$ lsmod | grep my_pci_stage3
my_pci_stage3          16384  0
ubuntu@ubuntu:~/pci_phase3$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: my_pci_stage3
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase3$ sudo dmesg | tail -30
[   28.934270] qnoc-sc7280 1580000.interconnect: Timed out. Forcing sync_state()
[   28.934272] qnoc-sc7280 1740000.interconnect: Timed out. Forcing sync_state()
[   28.934293] qnoc-sc7280 9100000.interconnect: Timed out. Forcing sync_state()
[   36.102853] refgen: disabling
[   84.631291] fbcon: Taking over console
[  176.284349] systemd-journald[669]: /var/log/journal/ff426f8f1c114d7061addb6c48e79742/user-1000.journal: Journal file uses a different sequence number ID, rotating.
[  176.585212] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  176.585234] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  176.585242] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  176.585248] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  176.586086] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  176.586103] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  176.586111] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  176.586119] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  176.597690] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  176.597737] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  176.597746] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  176.597755] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  176.599060] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(928): QC_IMAGE_VERSION_STRING=video-firmware.2.4.2-1bb9d0b4565acc9b6c035192fdacbad5a65effbf
[  176.599081] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(929): IMAGE_VARIANT_STRING=PROD
[  176.599090] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(930): OEM_IMAGE_VERSION_STRING=hw-rahutrip-hyd
[  176.599099] msm_vidc:     fw: <VFW_E:HostDr:unkn:--------:--> VenusHostDriver_ParseC2Command(931): BUILD_TIME: Nov 12 2024 11:44:20
[  273.857953] r8169 0001:01:00.0 enP1p1s0: Link is Down
[  287.811450] my_pci_stage3 0001:01:00.0: Stage 3: PCI probe started
[  287.811535] my_pci_stage3 0001:01:00.0: Stage 3: BAR2 mapped successfully
[  287.811545] my_pci_stage3 0001:01:00.0: Stage 3: TxConfig = 0x57100f80, XID = 0x541
[  287.811548] my_pci_stage3 0001:01:00.0: Stage 3: RTL8168H confirmed
[  287.811550] my_pci_stage3 0001:01:00.0: Stage 3: resetting hardware
[  287.811568] my_pci_stage3 0001:01:00.0: Stage 3: hardware reset complete
[  287.811570] my_pci_stage3 0001:01:00.0: Stage 3: hardware ready
ubuntu@ubuntu:~/pci_phase3$ sudo dmesg | grep "Stage 3"
[  287.811450] my_pci_stage3 0001:01:00.0: Stage 3: PCI probe started
[  287.811535] my_pci_stage3 0001:01:00.0: Stage 3: BAR2 mapped successfully
[  287.811545] my_pci_stage3 0001:01:00.0: Stage 3: TxConfig = 0x57100f80, XID = 0x541
[  287.811548] my_pci_stage3 0001:01:00.0: Stage 3: RTL8168H confirmed
[  287.811550] my_pci_stage3 0001:01:00.0: Stage 3: resetting hardware
[  287.811568] my_pci_stage3 0001:01:00.0: Stage 3: hardware reset complete
[  287.811570] my_pci_stage3 0001:01:00.0: Stage 3: hardware ready
ubuntu@ubuntu:~/pci_phase3$ sudo rmmod my_pci_stage3 
ubuntu@ubuntu:~/pci_phase3$ lsmod | grep my_pci_stage3
ubuntu@ubuntu:~/pci_phase3$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase3$ 


