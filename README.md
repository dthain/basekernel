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

This project is led by Prof. Douglas Thain at the University of Notre Dame.
A variety of students have contributed to the code, including
Jack Mazanec, Douglas Smith, Ethan Williams, Jon Westhoff, and Kevin Wern.

To learn more, see the [Basekernel Wiki](https://github.com/dthain/basekernel/wiki).

## Quick Start Instructions

If you are building on a Linux-X86 machine
and have the QEMU virtual machine installed,
then it could be as easy as this to build and run:

```
git clone https://github.com/dthain/basekernel
cd basekernel
make
qemu-system-i386 -cdrom basekernel.iso
```

You should see something like this:

<img src=screenshot.png align=center>

After some initial boot messages, you will see the kernel shell prompt.
This allows you to take some simple actions before running the first
user level program.  To automatically boot the first available filesystem,
whether on cdrom or harddisk, run <tt>automount</tt>:

<pre>
automount
</pre>

Then use the <tt>list</tt> command to examine the root directory:

<pre>
list /
</pre>

And use the <tt>run</tt> command to run a program.
For example, <tt>/bin/manager.exe</tt> runs a simple
four-pane window manager with several tasks in parallel.
(Use TAB to switch focus between programs, and tilde
to cancel out of the window manager.)

<pre>
run /bin/manager.exe
</pre>

<img src=screenshot-windows.png align=center>

Press TAB to change the focus between windows,
and you can interact with each process in turn.

## Not So Quick Start Instructions

If you are building on any other type of machine (not Linux or not-X86)
then you will need to build a cross-compiler toolchain first:

1 - Run `./build-cross-compiler.sh` which will download and build the necessary compiler, linker,
debugger, etc to create and run 32-bit X86 code.  **Be patient: this could take an hour or longer to complete.**

2 - Double-check that the cross-compiler was built correctly:
```
i686-elf-gcc --version
```

3 - Modify `Makefile.config` and set `CROSS_COMPILE=true`

4 - Build with `make` and then proceed as normal.