# IPMI

```bash

modprobe ipmi_devintf
modprobe ipmi_si

ipmitool -I open user list 1

ID  Name             Callin  Link Auth  IPMI Msg   Channel Priv Limit
2   ADMIN            false   false      true       ADMINISTRATOR

ipmitool -I open user set password 2

ipmitool lan print 1

ipmitool lan set 1 ipsrc static
ipmitool lan set 1 ipaddr 192.168.1.211
ipmitool lan set 1 netmask 255.255.255.0
ipmitool lan set 1 defgw ipaddr 192.168.1.254
ipmitool lan set 1 defgw macaddr 00:0e:0c:aa:8e:13
ipmitool lan set 1 arp respond on
ipmitool lan set 1 auth ADMIN MD5
ipmitool lan set 1 access on

```
