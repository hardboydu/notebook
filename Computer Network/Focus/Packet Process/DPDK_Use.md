# DPDK USE

## Compile Install

```bash
version dpdk-17.05.2

reference doc/build-sdk-quick.txt

make config T=x86_64-native-linuxapp-gcc

vim build/.config
CONFIG_RTE_LIBEAL_USE_HPET=y
CONFIG_RTE_BUILD_SHARED_LIB=y

make
make install DESTDIR=/opt/dpdk-17.05.2 prefix=/
```

## Run

```bash
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge
echo 1024 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 1024 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages

modprobe uio
insmod <dpdk path>/igb_uio.ko

<dpdk path>/dpdk-devbind --bind=igb_uio enp4s0f0 enp4s0f1
```

## Development

```bash
CFLAGS += -march=corei7
```

## pktgen dpdk

```bash
app/app/x86_64-native-linuxapp-gcc/app/pktgen -c f -n 4 --proc-type auto --socket-mem 256 -- -P -m "1.0,1.1"

-c 3 使用三个core
-m "1.0,1.1" 1 core控制port 0,1 core控制port 1，core 0一定要预留出来给 pktgen display processing
```
