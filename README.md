# spfs

A simple file system for NVMe SSD developed with SPDK.

## Environment

First install QEMU on your PC.

```bash
leohh@pc:~$ wget https://download.qemu.org/qemu-7.1.0.tar.xz

leohh@pc:~$ tar -xvJf qemu-7.1.0.tar.xz

leohh@pc:~$ cd qemu-7.1.0

leohh@pc:~/qemu-7.1.0$ ./configure

leohh@pc:~/qemu-7.1.0$ make

leohh@pc:~/qemu-7.1.0$ sudo make install

leohh@pc:~/qemu-7.1.0$ qemu-system-x86_64 --version
QEMU emulator version 7.1.0
Copyright (c) 2003-2022 Fabrice Bellard and the QEMU Project developers
```

Download Ubuntu 22.04 image.

```bash
leohh@pc:~$ wget https://releases.ubuntu.com/22.04.1/ubuntu-22.04.1-live-server-amd64.iso
```

Create virtual disk file.

```bash
leohh@pc:~$ qemu-img create -f qcow2 ubuntu.qcow2 40G
```

Start QEMU with the following command, then follow the prompts to install Ubuntu.

```bash
leohh@pc:~$ sudo qemu-system-x86_64 --enable-kvm -m 8G -smp 2 -boot order=dc -hda ubuntu.qcow2 -cdrom ~/ubuntu-22.04.1-live-server-amd64.iso
```

Create virtual NVMe SSD file.

```bash
qemu-img create -f qcow2 nvme.qcow2 128M
```

Start QEMU again. Remember to change the IP to yours so you can connect to QEMU using SSH.

```bash
leohh@pc:~$ sudo qemu-system-x86_64 -name qemucsd -m 8G --enable-kvm -cpu host -smp 4 \
-hda ./ubuntu.qcow2 \
-net user,hostfwd=tcp:192.168.3.13:7777-:22,hostfwd=tcp:192.168.3.13:2222-:2000 -net nic \
-drive file=nvme.qcow2,if=none,id=nvm \
-device nvme,serial=deadbeef,drive=nvm
```

## Installation

Do the following in QEMU.

```bash
leohh@vm:~$ git clone https://github.com/Leohh123/spfs.git --recursive

leohh@vm:~$ cd spfs/spdk

leohh@vm:~/spfs/spdk$ sudo ./scripts/pkgdep.sh --all

leohh@vm:~/spfs/spdk$ ./configure

leohh@vm:~/spfs/spdk$ make

leohh@vm:~/spfs/spdk$ ./test/unit/unittest.sh
...
=====================
All unit tests passed
=====================

leohh@vm:~/spfs/spdk$ nvme list
Node                  SN                   Model                                    Namespace Usage                      Format           FW Rev  
--------------------- -------------------- ---------------------------------------- --------- -------------------------- ---------------- --------
/dev/nvme0n1                                                                        1           0.00   B /   0.00   B      1   B +  0 B   

leohh@vm:~/spfs/spdk$ sudo ./scripts/setup.sh
0000:00:04.0 (1b36 0010): nvme -> uio_pci_generic

leohh@vm:~/spfs/spdk$ nvme list
Node                  SN                   Model                                    Namespace Usage                      Format           FW Rev  
--------------------- -------------------- ---------------------------------------- --------- -------------------------- ---------------- --------

leohh@vm:~/spfs/spdk$ sudo ./build/examples/hello_world 
TELEMETRY: No legacy callbacks, legacy socket not created
Initializing NVMe Controllers
Attaching to 0000:00:04.0
Attached to 0000:00:04.0
  Namespace ID: 1 size: 10GB
Initialization complete.
INFO: using host memory buffer for IO
Hello world!

leohh@vm:~/spfs/spdk$ cd ..

leohh@vm:~/spfs$ make
...
make[2]: Leaving directory '/home/leohh/spfs/test/size'
make[1]: Leaving directory '/home/leohh/spfs/test'
```

## Usage

Header files:

- For block read and write, use `include/block.h` ;
- For basic file manipulation functions, use `include/fs.h` ;
- For file functions compatible with POSIX (fopen, fread, etc.), use `include/io.h` .

Executable: source code in `/src/bin` , and outputs in `/build/bin` (ls, mkdir, cat, etc.). Example:

```bash
leohh@vm:~/spfs/build/bin$ sudo ./ls /
TELEMETRY: No legacy callbacks, legacy socket not created
---------------- ls 1 start ----------------
<DIR>           a               Blkno: 2
<FILE>          c.txt           Blkno: 4
---------------- ls 1 end ----------------
```

Tests: source code in `/test` , and outputs in `/build/test` . Example:
```bash
leohh@vm:~/spfs/build/test$ sudo ./test_file 
TELEMETRY: No legacy callbacks, legacy socket not created
0123
89ab
Test passed.
```