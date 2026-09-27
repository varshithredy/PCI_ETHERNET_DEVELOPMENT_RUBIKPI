ubuntu@ubuntu:~$ cd pci_phase4/
ubuntu@ubuntu:~/pci_phase4$ ls
Makefile        my_pci_driver.c  my_pci_stage4.ko     my_pci_stage4.mod.o
Module.symvers  my_pci_driver.h  my_pci_stage4.mod    my_pci_stage4.o
modules.order   my_pci_driver.o  my_pci_stage4.mod.c
ubuntu@ubuntu:~/pci_phase4$ cd ~/pci_phase4

make clean
make
make -C /lib/modules/6.8.0-1080-qcom/build M=/home/ubuntu/pci_phase4 clean
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1080-qcom'
  CLEAN   /home/ubuntu/pci_phase4/Module.symvers
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1080-qcom'
make -C /lib/modules/6.8.0-1080-qcom/build M=/home/ubuntu/pci_phase4 modules
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1080-qcom'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  CC [M]  /home/ubuntu/pci_phase4/my_pci_driver.o
  LD [M]  /home/ubuntu/pci_phase4/my_pci_stage4.o
  MODPOST /home/ubuntu/pci_phase4/Module.symvers
  CC [M]  /home/ubuntu/pci_phase4/my_pci_stage4.mod.o
  LD [M]  /home/ubuntu/pci_phase4/my_pci_stage4.ko
  BTF [M] /home/ubuntu/pci_phase4/my_pci_stage4.ko
Skipping BTF generation for /home/ubuntu/pci_phase4/my_pci_stage4.ko due to unavailability of vmlinux
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1080-qcom'
ubuntu@ubuntu:~/pci_phase4$ ls -lh my_pci_stage4.ko
-rw-rw-r-- 1 ubuntu ubuntu 451K Sep 22 11:01 my_pci_stage4.ko
ubuntu@ubuntu:~/pci_phase4$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: r8169
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase4$ echo 0001:01:00.0 | sudo tee /sys/bus/pci/drivers/r8169/unbind
0001:01:00.0
ubuntu@ubuntu:~/pci_phase4$ sudo insmod ./my_pci_stage4.ko
ubuntu@ubuntu:~/pci_phase4$ lspci -nnk -s 0001:01:00.0
0001:01:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8161] (rev 15)
	Subsystem: Realtek Semiconductor Co., Ltd. TP-Link TG-3468 v4.0 Gigabit PCI Express Network Adapter [10ec:8168]
	Kernel driver in use: my_pci_stage4
	Kernel modules: r8169
ubuntu@ubuntu:~/pci_phase4$ sudo dmesg | grep "Stage 4"
[ 2169.939798] my_pci_stage4 0001:01:00.0: Stage 4: probe started
[ 2169.939888] my_pci_stage4 0001:01:00.0: Stage 4: BAR2 mapped
[ 2169.939898] my_pci_stage4 0001:01:00.0: Stage 4: TxConfig = 0x57100f80, ID = 0x541
[ 2169.939903] my_pci_stage4 0001:01:00.0: Stage 4: device check passed
[ 2169.939906] my_pci_stage4 0001:01:00.0: Stage 4: resetting device
[ 2169.939923] my_pci_stage4 0001:01:00.0: Stage 4: reset complete
[ 2169.939972] my_pci_stage4 0001:01:00.0: Stage 4: MAC address 58:04:4f:67:5d:69
[ 2169.939975] my_pci_stage4 0001:01:00.0: Stage 4: device is ready with MAC 58:04:4f:67:5d:69
ubuntu@ubuntu:~/pci_phase4$ sudo rmmod my_pci_stage4

