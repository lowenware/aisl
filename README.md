# AISL
Asynchronous Internet Server Library

## Installation on CentOS 7 / RedHat 7

1. Add repository
```
cd ~
wget http://lowenware.com/rpm/redhat-7/lowenware.repo
sudo mv lowenware.repo /etc/yum.repos.d/
```

2. Import GPG key
```
sudo rpm --import http://lowenware.com/rpm/RPM-GPG-KEY-Lowenware
```

3. Install
```
sudo yum install aisl aisl-devel
```

## Installation on OpenSUSE Tumbleweed

1. Add repository
```
sudo zypper ar http://lowenware.com/rpm/opensuse-tumbleweed/lowenware.repo
```

2. Import GPG key
```
sudo rpm --import http://lowenware.com/rpm/RPM-GPG-KEY-Lowenware
```

3. Install
```
sudo zypper install aisl aisl-devel
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
