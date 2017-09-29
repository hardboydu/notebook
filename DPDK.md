# Compile Install

```
version dpdk-17.05.2

reference doc/build-sdk-quick.txt

make config T=x86_64-native-linuxapp-gcc

vim build/.config
CONFIG_RTE_LIBEAL_USE_HPET=y
CONFIG_RTE_BUILD_SHARED_LIB=y

make
make install DESTDIR=/opt/dpdk-17.05.2 prefix=/
```

# Run

```
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge
echo 1024 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 1024 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages

modprobe uio
insmod <dpdk path>/igb_uio.ko

<dpdk path>/dpdk-devbind --bind=igb_uio enp4s0f0 enp4s0f1
```

# Development

```
CFLAGS += -march=corei7
```
