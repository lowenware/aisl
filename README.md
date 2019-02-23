# AISL
Asynchronous Internet Server Library

## Installation on CentOS 7 / RedHat 7

1. Add repository
```
sudo curl -o /etc/yum.repos.d/lowenware.repo https://lowenware.com/rpm/redhat-7/lowenware.repo
```

2. Import GPG key
```
sudo rpm --import https://lowenware.com/rpm/RPM-GPG-KEY-Lowenware
```

3. Install
```
sudo yum install aisl aisl-devel
```

## Installation from sources on any distro

1. Configuration
```
cmake -DCMAKE_INSTALL_PREFIX=/usr/local
```

2. Compilation
```
make
```

3. Installation
```
sudo make install
```
