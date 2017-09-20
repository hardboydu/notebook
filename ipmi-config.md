modprobe ipmi_devintf
modprobe ipmi_si

ipmitool -I open user list 1

ID  Name             Callin  Link Auth  IPMI Msg   Channel Priv Limit
2   ADMIN            false   false      true       ADMINISTRATOR

ipmitool -I open user set password 2
