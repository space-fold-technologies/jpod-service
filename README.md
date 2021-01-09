# JPOD engine
[jpod engine](https://github.com/space-fold-technologies/jpod-engine) is a Core Engine for the easy FreeBSD jail composition and management.

### Requirements
Before moving into compiling the project, we need to met the following conditions.
  - Be sure you're using `zfs`.
  - Be sure you have enabled linux binary compatibility. 

In your current session you can enable it by typing:
```sh
# kldload linux64
```
and to make it persistent, you can write the following line into your `/etc/rc.conf`
```sh
linux_enable="YES"
```

### Dependencies

In order to install the jpod-engine in your system you need `cmake`, `boost` and a `C++ 17` compiler (like `clang 9`). In FreeBSD install the following packages:
```sh
$ pkg install boost-libs llvm90
```

### Make an Alpine Linux Snapshot

First we need to create a mountpoint and a zfs data set.
```sh
doas zfs create -o mountpoint=/usr/local/images zroot/images
doas zfs create zroot/images/alpine
```
Now we need to get a minimal alpine-linux image. (using x86_64 arch here).
```sh
$ wget -c https://dl-cdn.alpinelinux.org/alpine/v3.12/releases/x86_64/alpine-minirootfs-3.12.2-x86_64.tar.gz
$ doas tar -xvzf alpine-minirootfs-3.12.2-x86_64.tar.gz -C /usr/local/images/alpine
```
Next we need to get in and do updates to the system
```sh
chroot /usr/local/images/alpine /bin/sh
```
Exit chroot and end the snapshot process
```sh
doas zfs snapshot zroot/images/alpine@latest
doas zfs clone zroot/images/alpine@latest zroot/images/alo
```

### Compile Jpod engine
Do a quick git clone to get the project in a local directory.
```sh
git clone -b jail-operations-implementation https://github.com/space-fold-technologies/jpod-engine.git
```
Do a cd into the cloned project. We need to recursively update the gitmodules.
```sh
git submodule update --init --recursive
```
Once its done, we need to set our snapshot path into the `src/main.cc` file (this will be automated in next versions):
```sh
composition.snapshot_path = "/usr/local/images/alo";
```
Now we can perform the building step. Run the following command:
```sh
mkdir build && cd build && cmake .. && make && cd bin
```
And we should have a `jpod-engine` binary. We need to run it as root, either with `doas` or `sudo`.
```sh
doas ./jpod-engine
```
If we check the `jls` command now whe should have the following:
```sh
JID     IP Address      Hostname    Path
  1     127.0.0.1       developer   /usr/local/images/alo
```

### Entering the shell
We now can enter the jail shell using the `JID` (1 in this case):
```sh
doas jexec 1 /bin/sh 
```

To exit it we can just run `exit`.

### Killing the jail
```sh
doas jail -r ${JID}
```
