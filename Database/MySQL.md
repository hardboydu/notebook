# MySQL

## 初始化空库

```
cd /opt
mkdir data
chown mysql:mysql data
cd data
mysqld --initialize-insecure
```

## 启动

```
mysqld_safe --datadir=/opt/data --bind-address=192.168.110.105
```

## root 远程登录

```
mysql
use mysql;
GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'root' WITH GRANT OPTION;
```
