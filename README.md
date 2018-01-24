# The Basekernel Operating System Kernel

Basekernel is a simple operating system kernel for research, teaching, and fun.

Basekernel is *not* a complete operating system, but it is a starting
point for those who wish to study and develop new operating system code.
If you want to build something *different* than Windows, Linux, or Multics,
Basekernel may be a good place to try out your new ideas.

Basekernel can boot an Intel PC-compatible virtual machine in 32-bit protected
mode, with support for VESA framebuffer graphics, ATA hard disks, ATAPI optical
devices, process management, memory protection, simple graphics, and basic filesystem.
From there, it's your job to write user-level programs and expand the system.

To be clear, this is raw low-level code, not a production system.
If you want to hack code and learn about operating system, you will like Basekernel.
If you are looking for a new OS to run on your laptop, then this is not what you want.

This project is led by Prof. Douglas Thain at the University of Notre Dame,
with contributions from students, particularly Jon Westhoff and Kevin Wern.

To learn more, see the [Basekernel Wiki](https://github.com/dthain/basekernel/wiki).

## Quick Start Instructions

```
git clone https://github.com/dthain/basekernel
cd basekernel
./build-cross-compiler.sh
export PATH=`pwd`/cross/bin:$PATH
cd src
make
qemu-system-i386 -cdrom basekernel.iso
```

And you should see something like this:

<img src=screenshot.png align=center>

After some initial boot messages, you will see the kernel shell prompt.
This allows you to take some simple actions before running the first
user level program.  For example, read the boot messages to see
which ata unit the cdrom is mounted on.  Then, use the <tt>mount</tt> command
to mount the cdrom filesystem on that unit:

<pre>
mount 2
</pre>

Use the <tt>list</tt> command to examine the root directory:

<pre>
list /
</pre>

And use the <tt>run</tt> command to run a program:

<pre>
run /SAVER.EXE
</pre>

