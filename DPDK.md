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
