# QEMU AARCH64

## Build QEMU with AARCH64

```sh
../configure --prefix=$PREFIX/$QEMU \
    --target-list=x86_64-softmmu,x86_64-linux-user,aarch64-softmmu,aarch64_be-linux-user,aarch64-linux-user,arm-softmmu,armeb-linux-user,arm-linux-user \
    --enable-user \
    --enable-linux-user \
    --enable-bsd-user \
    --enable-docs \
    --enable-curses \
    --disable-gtk \
    --disable-gnutls \
    --disable-gcrypt
```

## Install Debian 9.9

### Reference

* [Running an ISO installer image for arm64 (AArch64) using QEMU and KVM](http://www.redfelineninja.org.uk/daniel/2018/02/running-an-iso-installer-image-for-arm64-aarch64-using-qemu-and-kvm/)
* [Using UEFI in QEMU/KVM (AArch64 + AArch32)](https://web.archive.org/web/20180218102845/https://wiki.linaro.org/LEG/UEFIforQEMU)
* [Virtualize UEFI on ARM using QEMU](https://designprincipia.com/virtualize-uefi-on-arm-using-qemu/)
* [Device 'vfio-pci' could not be initialized](https://www.linux.org.ru/forum/general/12459285)

下载 BIOS 固件

[QEMU_EFI.img.gz](http://snapshots.linaro.org/components/kernel/leg-virt-tianocore-edk2-upstream/latest/QEMU-AARCH64/RELEASE_GCC5/QEMU_EFI.img.gz)

创建虚拟磁盘

```sh
qemu-img create -f qcow2 debian.img 128G
qemu-img create -f qcow2 varstore.img 64M
```

安装操作系统

```sh
#!/usr/bin/env bash

qemu-system-aarch64 \
    -cpu cortex-a57 -M virt -m 4096 -nographic \
    -drive if=pflash,format=raw,file=QEMU_EFI.img \
    -drive if=pflash,file=varstore.img \
    -drive if=virtio,file=ubuntu.img \
    -drive if=virtio,format=raw,file=debian-9.9.0-arm64-netinst.iso
```

当提示 Load CD-ROM drivers from removable media 时，做如下操作

* Load CD-ROM drivers from removable media?: Select No
* Manually select a CD-ROM module and device?: Select Yes
* Module needed for accessing the CD-ROM: Select none
* Device file for accessing the CD-ROM: Enter /dev/vdb and press Continue

启动操作系统

```sh
qemu-system-aarch64 \
    -M vexpress-a9 -cpu cortex-a57 -smp 8 -M virt -m 8192 -nographic \
    -drive if=pflash,format=raw,file=QEMU_EFI.img \
    -drive if=pflash,file=varstore.img \
    -drive if=virtio,file=debian.img \
    -netdev tap,id=arm0,br=bridge0,helper="/opt/devrte/runtime/qemu-4.0.0/libexec/qemu-bridge-helper" \
    -device virtio-net-pci,netdev=arm0,mac="52:54:00:12:34:56" \
    -device vfio-pci,host=06:00.0,id=hostdev2 \
    -device vfio-pci,host=06:00.1,id=hostdev3 \
```

对于PCI 直通的设备需要实现将其挂接到 vfio驱动上

脚本 `pci.sh` : 

```sh
#!/bin/sh

for dev in $@; do
    vendor=$(cat /sys/bus/pci/devices/$dev/vendor)
    device=$(cat /sys/bus/pci/devices/$dev/device)
    if [ -e /sys/bus/pci/devices/$dev/driver ]; then
        echo $dev > /sys/bus/pci/devices/$dev/driver/unbind
    fi
    echo $vendor $device > /sys/bus/pci/drivers/vfio-pci/new_id
done
```

执行此脚本

```sh
sh pci.sh 0000:06:00.0 0000:06:00.1
```
