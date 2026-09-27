ubuntu@ubuntu:~/pci_phase8$ vi Makefile 
ubuntu@ubuntu:~/pci_phase8$ rm my_pci_stage8.h 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8.h 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_net.c 
ubuntu@ubuntu:~/pci_phase8$ rm my_pci_stage8_net.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_net.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_phy.c 
ubuntu@ubuntu:~/pci_phase8$ rm my_pci_stage8_phy.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_phy.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_dma.c 
ubuntu@ubuntu:~/pci_phase8$ rm my_pci_stage8_firmware.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_firmware.c 
ubuntu@ubuntu:~/pci_phase8$ rm my_pci_stage8_dma.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_dma.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_main.c 
ubuntu@ubuntu:~/pci_phase8$ rm my_pci_stage8_main.c 
ubuntu@ubuntu:~/pci_phase8$ vi my_pci_stage8_main.c 
ubuntu@ubuntu:~/pci_phase8$ ls
Makefile  my_pci_stage8.h      my_pci_stage8_firmware.c  my_pci_stage8_net.c
logs.md   my_pci_stage8_dma.c  my_pci_stage8_main.c      my_pci_stage8_phy.c
ubuntu@ubuntu:~/pci_phase8$ make 
make -C /lib/modules/6.8.0-1084-qcom/build M=/home/ubuntu/pci_phase8 modules
make[1]: Entering directory '/usr/src/linux-headers-6.8.0-1084-qcom'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  CC [M]  /home/ubuntu/pci_phase8/my_pci_stage8_main.o
  CC [M]  /home/ubuntu/pci_phase8/my_pci_stage8_firmware.o
  CC [M]  /home/ubuntu/pci_phase8/my_pci_stage8_net.o
  CC [M]  /home/ubuntu/pci_phase8/my_pci_stage8_dma.o
  CC [M]  /home/ubuntu/pci_phase8/my_pci_stage8_phy.o
  LD [M]  /home/ubuntu/pci_phase8/my_pci_stage8.o
  MODPOST /home/ubuntu/pci_phase8/Module.symvers
  CC [M]  /home/ubuntu/pci_phase8/my_pci_stage8.mod.o
  LD [M]  /home/ubuntu/pci_phase8/my_pci_stage8.ko
  BTF [M] /home/ubuntu/pci_phase8/my_pci_stage8.ko
Skipping BTF generation for /home/ubuntu/pci_phase8/my_pci_stage8.ko due to unavailability of vmlinux
make[1]: Leaving directory '/usr/src/linux-headers-6.8.0-1084-qcom'
ubuntu@ubuntu:~/pci_phase8$ sudo ip link set eth0 down 2>/dev/null || true
sudo rmmod my_pci_stage8 2>/dev/null || true
ubuntu@ubuntu:~/pci_phase8$ sudo ip link set enP1p1s0 down 2>/dev/null || true
ubuntu@ubuntu:~/pci_phase8$ sudo sh -c 'echo 0001:01:00.0 > /sys/bus/pci/drivers/r8169/unbind'
ubuntu@ubuntu:~/pci_phase8$ sudo dmesg -C
ubuntu@ubuntu:~/pci_phase8$ sudo insmod ./my_pci_stage8.ko
ubuntu@ubuntu:~/pci_phase8$ sudo ip link set eth0 up
ubuntu@ubuntu:~/pci_phase8$ sudo dmesg | grep -iE 'my_pci_stage8|Stage 8'
[ 2356.166072] my_pci_stage8 0001:01:00.0: Stage 8: probe started
[ 2356.166191] my_pci_stage8 0001:01:00.0: Stage 8: BAR2 mapped
[ 2356.166201] my_pci_stage8 0001:01:00.0: Stage 8: TxConfig=0x57100f80 XID=0x541
[ 2356.166204] my_pci_stage8 0001:01:00.0: Stage 8: resetting device
[ 2356.166387] my_pci_stage8 0001:01:00.0: Stage 8: firmware rtl8168h-2_0.0.2 02/26/15, actions=211
[ 2356.185261] my_pci_stage8 0001:01:00.0: Stage 8: PHY completion check passed, BMCR=0x1840
[ 2356.185370] my_pci_stage8 0001:01:00.0: Stage 8: DMA rings allocated TX=64 RX=64 descriptors
[ 2356.185375] my_pci_stage8 0001:01:00.0: Stage 8: TX DMA=0x00000000ff8a9000 RX DMA=0x00000000ff8a8000
[ 2356.185732] my_pci_stage8 0001:01:00.0: Stage 8: network device registered as eth0
[ 2356.185737] my_pci_stage8 0001:01:00.0: Stage 8: DMA datapath prepared
[ 2356.244091] my_pci_stage8 0001:01:00.0: Stage 8: starting PHY configuration and auto-negotiation
[ 2356.256119] my_pci_stage8 0001:01:00.0: Stage 8: PHY BMCR=0x1240, auto-negotiation started
[ 2360.004435] my_pci_stage8 0001:01:00.0: Stage 8: PHY link not detected yet, BMSR=0x7989
[ 2360.004535] my_pci_stage8 0001:01:00.0: Stage 8: DMA engine started
[ 2360.004551] my_pci_stage8 0001:01:00.0: Stage 8: network interface opened
ubuntu@ubuntu:~/pci_phase8$ cat /sys/class/net/eth0/carrier
ip -br link show eth0
1
eth0             UP             58:04:4f:67:5d:69 <BROADCAST,MULTICAST,UP,LOWER_UP> 
ubuntu@ubuntu:~/pci_phase8$ 


