# SSH

# SSH Keygen

```sh
ssh-keygen
cat id_rsa.pub >> ~/.ssh/authorized_keys
chmod 644 .ssh/authorized_keys
```

```conf
/etc/ssh/sshd_config
RSAAuthentication yes
PubkeyAuthentication yes
```