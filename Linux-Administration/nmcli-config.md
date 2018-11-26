# nmcli

```sh
nmcli con mod eno1 ip4 192.168.6.102/24 ipv4.method manual
```

```sh
nmcli con mod eno1 +ipv4.address 192.168.8.34/24 ipv4.gateway 192.168.8.1
```

```sh
nmcli con mod eno1 ipv4.dns 8.8.8.8
```