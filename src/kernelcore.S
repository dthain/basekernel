# Copyright (C) 2015 The University of Notre Dame
# This software is distributed under the GNU General Public License.
# See the file LICENSE for details.

#include "memorylayout.h"
	
# _start is the initial entry point for the kernel
# Note that we start here merely because it comes
# first in the link order.  The name _start is there
# merely to keep the linker happy.

.code16
.text
.global _start
_start:

# First, jump to the real code start,
# skipping over the immediate data that follows
	jmp	realstart

# At a fixed offset, place an integer that contains
# the length of the kernel in bytes.  This is used
# by the bootblock code to figure out how many sectors to load.
	
.org KERNEL_SIZE_OFFSET
.global kernel_size
kernel_size:
	.long	_end-_start
		
realstart:
	
# While we are briefly still in 16-bit real mode,
# it is safe to call the BIOS to set things up.

# Reset the disk system in order to quiet motors
# and turn off any pending interrupts.
		
	mov	$0,%ah
	int	$0x13

# Turn off the screen cursor, because the
# console will have its own.

	mov	$1,%ah
	mov	$0,%cl
	mov	$0x20,%ch
	int	$0x10

# Get the amount of memory above 1MB and save it for later.
# There are two ways to do this:

# BIOS call 0xe801 measures memory as follows:	
# %ax returns memory above 1MB in 1KB increments, maximum of 16MB.
# %bx returns memory above 64MB in 64KB increments, maximum of 4GB.
# However, this call is relatively new, so if it
# fails, we fall back on memtest2 below.

memtest1:	
	clc
	mov	$0, %bx
	mov	$0xe801,%ax
	int	$0x15
	jc	memtest2
	
	shr	$10, %ax
	shr	$4, %bx
	add	%ax, %bx
	mov	%bx, total_memory-_start
	jmp	memdone

# BIOS call 0x0088 measures memory as follows:
# %ax returns memory above 1MB in 1KB increments, maxiumum of 64MB.

memtest2:
	clc
	mov	$0, %ax
	mov	$0x88, %ah
	int	$0x15
	shr	$10, %ax
	inc	%ax
	mov	%ax, total_memory-_start
memdone:	

# Now, set the video mode using VBE interrupts.
# Keep trying until we find one that works.

# These are documented on page 30 of the VESA-BIOS manual:
# interrupt 0x10
# ax = 0x4f02 "Set VBE Mode"
# bx = mode
#    D0-8   = Mode Number
#    D9-10  = Reserved (must be 0)
#    D11    = 0 Use current default refresh rate.
#    D12-13 = 0 Reserved
#    D14    = 0 Use windowed frame buffer model.
#           = 1 Use linear frame buffer model.
#    D15    = 0 Clear display memory.
#    ES:DI  = Pointer to CRCTCInfoBlock structure.

jmp video1024
	
video1280:
 	mov	$0x4f02, %ax
 	mov	$0x411b, %bx
	int	$0x10
	cmp	$0x004f, %ax
	je	videodone
video1024:
 	mov	$0x4f02, %ax
 	mov	$0x4118, %bx
	int	$0x10
	cmp	$0x004f, %ax
	je	videodone
video800:
	mov	$0x4f02, %ax
 	mov	$0x4115, %bx
	int	$0x10
	cmp	$0x004f, %ax
	je	videodone
video640:
	mov	$0x4f02, %ax
 	mov	$0x4112, %bx
	int	$0x10
	cmp	$0x004f, %ax
	je	videodone
video640_lowcolor:
	mov	$0x4f02, %ax
	mov	$0x4111, %bx
	int	$0x10
	cmp	$0x004f, %ax
	je	videodone

videofailed:
	mov	$videomsg, %esi
	call	bios_putstring
	jmp	halt

videodone:	

# After all that, query the video mode and
# figure out the dimensions and the frame
# buffer address.  The set mode is still in bx.

	mov	%ds, %ax		# Set up the extra segment
	mov	%ax, %es		# with the data segment

	mov	$(video_info-_start),%di
	mov	$0x4f01, %ax
	mov	%bx, %cx
	int	$0x10

# In order to use video resolutions higher than 640x480,
# we must enable the A20 address line. The following
# code works on motherboards with "FAST A20", which should
# be everything since the IBM PS/2
inb $0x92, %al
orb $2, %al
outb %al, $0x92
	
# Finally, we are ready to enter protected mode.
# To do this, we disable interrupts so that
# handlers will not see an inconsistent state.
# We then load the new interrupt and descriptor
# tables, which are given below.  Then, we
# enable the protection bit, and load the
# segment selectors into the appropriate registers.
# Finally, we make a long jump to main,
# atomically loading the new code segment and
# starting the kernel.

	cli				# clear interrupts
	lidt	(idt_init-_start)	# load the interrupt table
	lgdt	(gdt_init-_start)	# load the descriptor table
	mov	%cr0, %eax		# get the status word
	or	$0x01, %eax		# turn on the P bit
	mov	%eax, %cr0		# store the status word
					# (we are now in protected mode)
	mov	$2*8, %ax		# selector two is flat 4GB data data
	mov	%ax, %ds		# set stack and data segments to selector two
	mov	%ax, %ss
	mov	$5*8, %ax		# set TSS to selector five
	ltr	%ax
	mov	$0, %ax			# unused segments are nulled out
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	$INTERRUPT_STACK_TOP, %sp    # set up initial C stack
	mov	$INTERRUPT_STACK_TOP, %bp    # set up initial C stack
	ljmpl	$(1*8), $(kernel_main)	     # jump to the C main!

# bios_putstring displays an ASCII string pointed to by %si,
# useful for basic startup messages or fatal errors.

bios_putstring:
	mov	(%si), %al
	cmp	$0, %al
	jz	bios_putstring_done
        call	bios_putchar
	inc	%si
	jmp	bios_putstring
bios_putstring_done:
        ret

# bios_putchar invokes the bios to display
# one character on the screen.
	
bios_putchar:
	push	%ax
	push	%bx
        mov	$14,%ah
        mov	$1,%bl
        int	$0x10
	pop	%bx
	pop	%ax
	ret

# The video_info structure is filled in by a BIOS
# call above, and is used to record the basic video
# layout for the graphics subsystem.  See page 30
# of the VBE specification for an explanation of this structure.

.align 4
video_info:
	.word	0
	.byte	0,0
	.word	0,0,0,0
	.long	0
.global video_xbytes
video_xbytes:
	.word	0
.global video_xres
video_xres:
	.word	0 
.global video_yres
video_yres:
	.word	0
	.byte	0,0,0,0,0,0,0,0,0
	.byte	0,0,0,0,0,0,0,0,0
.global video_buffer
video_buffer:
	.long	0
	.long	0
	.word	0
	.word	0
	.byte	0,0,0,0,0,0,0,0,0,0
	.long	0
.rept 190
	.byte	0
.endr

.align 4
videomsg:
	.asciz	"fatal error: couldn't find suitable video mode!\r\n"

###########################
# 32 BIT CODE BEGINS HERE #
###########################

# All code below this point is 32-bit code and data
# that is invoked by higher levels of the kernel from C code.
	
.code32

# Rebooting the machine is easy.
# Set up an invalid interrupt table, and the force an interrupt.
# The machine will triple-fault and reboot itself.

.global reboot
reboot:
	cli
	lidt	idt_invalid	
	int	$1


.global halt
halt:
	cli
	hlt
	jmp	halt

# This is the global descriptor table to be used by the kernel.
# Because we don't really want to use segmentation, we define
# very simple descriptors for global code and data and the TSS

.align 16
.global gdt
gdt:
	.word	0,0,0,0				# seg 0 - null
	.word	0xffff, 0x0000, 0x9a00, 0x00cf	# seg 1 - kernel flat 4GB code
	.word	0xffff, 0x0000, 0x9200, 0x00cf	# seg 2 - kernel flat 4GB data
	.word	0xffff, 0x0000, 0xfa00, 0x00cf	# seg 3 - user flat 4GB code
	.word	0xffff, 0x0000, 0xf200, 0x00cf	# seg 4 - user flat 4GB data
	.word	0x0068, (tss-_start),0x8901, 0x00cf  # seg 5 - TSS
	
# This is the initializer for the global descriptor table.
# It simply tells us the size and location of the table.

gdt_init:
	.word	gdt_init-gdt
	.long	gdt

# The TSS is a big task management structure used by the 386.
# We do not use the TSS, but simply rely on pushing variables
# around in stacks.  However, we need to use the TSS in order
# to initialize the stack pointer and segment for priv level 0

.align 16
.global tss	
tss:
	.long	0
.global interrupt_stack_pointer
interrupt_stack_pointer:	
	.long	INTERRUPT_STACK_TOP # initial interrupt stack ptr at 64 KB
	.long	2*8		    # use segment 2 for the interrupt stack
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	
.global total_memory
total_memory:
	.word	32
	
# First, the internal interrupts.
# Note that some already push their own detail
# code onto the stack.  For the others, we push
# a zero, just to get a common layout.

intr00: pushl $0 ; pushl $0  ; jmp intr_handler
intr01: pushl $0 ; pushl $1  ; jmp intr_handler
intr02: pushl $0 ; pushl $2  ; jmp intr_handler
intr03: pushl $0 ; pushl $3  ; jmp intr_handler
intr04: pushl $0 ; pushl $4  ; jmp intr_handler
intr05: pushl $0 ; pushl $5  ; jmp intr_handler
intr06: pushl $0 ; pushl $6  ; jmp intr_handler
intr07: pushl $0 ; pushl $7  ; jmp intr_handler
intr08:            pushl $8  ; jmp intr_handler
intr09: pushl $0 ; pushl $9  ; jmp intr_handler
intr10:            pushl $10 ; jmp intr_handler
intr11:            pushl $11 ; jmp intr_handler
intr12:            pushl $12 ; jmp intr_handler
intr13:            pushl $13 ; jmp intr_handler
intr14:            pushl $14 ; jmp intr_handler
intr15: pushl $0 ; pushl $15 ; jmp intr_handler
intr16: pushl $0 ; pushl $16 ; jmp intr_handler
intr17:            pushl $17 ; jmp intr_handler
intr18: pushl $0 ; pushl $18 ; jmp intr_handler
intr19: pushl $0 ; pushl $19 ; jmp intr_handler

# These interrupts are reserved, but could
# conceivably occur on the next processor model

intr20: pushl $0 ; pushl $20 ; jmp intr_handler
intr21: pushl $0 ; pushl $21 ; jmp intr_handler
intr22: pushl $0 ; pushl $22 ; jmp intr_handler
intr23: pushl $0 ; pushl $23 ; jmp intr_handler
intr24: pushl $0 ; pushl $24 ; jmp intr_handler
intr25: pushl $0 ; pushl $25 ; jmp intr_handler
intr26: pushl $0 ; pushl $26 ; jmp intr_handler
intr27: pushl $0 ; pushl $27 ; jmp intr_handler
intr28: pushl $0 ; pushl $28 ; jmp intr_handler
intr29: pushl $0 ; pushl $29 ; jmp intr_handler
intr30: pushl $0 ; pushl $30 ; jmp intr_handler
intr31: pushl $0 ; pushl $31 ; jmp intr_handler

# Now, the external hardware interrupts.

intr32: pushl $0 ; pushl $32 ; jmp intr_handler
intr33: pushl $0 ; pushl $33 ; jmp intr_handler
intr34: pushl $0 ; pushl $34 ; jmp intr_handler
intr35: pushl $0 ; pushl $35 ; jmp intr_handler
intr36: pushl $0 ; pushl $36 ; jmp intr_handler
intr37: pushl $0 ; pushl $37 ; jmp intr_handler
intr38: pushl $0 ; pushl $38 ; jmp intr_handler
intr39: pushl $0 ; pushl $39 ; jmp intr_handler
intr40: pushl $0 ; pushl $40 ; jmp intr_handler
intr41: pushl $0 ; pushl $41 ; jmp intr_handler
intr42: pushl $0 ; pushl $42 ; jmp intr_handler
intr43: pushl $0 ; pushl $43 ; jmp intr_handler
intr44: pushl $0 ; pushl $44 ; jmp intr_handler
intr45: pushl $0 ; pushl $45 ; jmp intr_handler
intr46: pushl $0 ; pushl $46 ; jmp intr_handler
intr47: pushl $0 ; pushl $47 ; jmp intr_handler
intr48: pushl $0 ; pushl $48 ; jmp intr_syscall

intr_handler:
	pushl	%ds
	pushl	%ebp		# push regs
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
	pushl	36(%esp)	# push interrupt code
	pushl	36(%esp)	# push interrupt number
	movl	$2*8, %eax	# switch to kernel data seg
	movl	%eax, %ds
	call	interrupt_handler
	addl	$4, %esp	# remove interrupt number
	addl	$4, %esp	# remove interrupt code
	jmp	intr_return
	
intr_syscall:
	pushl	%ds
	pushl	%ebp		# push regs
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%eax		# note these *are* the syscall args
	movl	$2*8, %eax	# switch to kernel data seg
	movl	%eax, %ds
	call	syscall_handler
	addl	$4, %esp	# remove the old eax
	jmp	syscall_return	

.global intr_return
intr_return:
	popl	%eax
syscall_return:	
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	popl	%ebp
	popl	%ds
	addl	$4, %esp	# remove interrupt num
	addl	$4, %esp	# remove detail code
	iret			# iret gets the intr context
			
.align 2
idt:
	.word	intr00-_start,1*8,0x8e00,0x0001
	.word	intr01-_start,1*8,0x8e00,0x0001
	.word	intr02-_start,1*8,0x8e00,0x0001
	.word	intr03-_start,1*8,0x8e00,0x0001
	.word	intr04-_start,1*8,0x8e00,0x0001
	.word	intr05-_start,1*8,0x8e00,0x0001
	.word	intr06-_start,1*8,0x8e00,0x0001
	.word	intr07-_start,1*8,0x8e00,0x0001
	.word	intr08-_start,1*8,0x8e00,0x0001
	.word	intr09-_start,1*8,0x8e00,0x0001
	.word	intr10-_start,1*8,0x8e00,0x0001
	.word	intr11-_start,1*8,0x8e00,0x0001
	.word	intr12-_start,1*8,0x8e00,0x0001
	.word	intr13-_start,1*8,0x8e00,0x0001
	.word	intr14-_start,1*8,0x8e00,0x0001
	.word	intr15-_start,1*8,0x8e00,0x0001
	.word	intr16-_start,1*8,0x8e00,0x0001
	.word	intr17-_start,1*8,0x8e00,0x0001
	.word	intr18-_start,1*8,0x8e00,0x0001
	.word	intr19-_start,1*8,0x8e00,0x0001
	.word	intr20-_start,1*8,0x8e00,0x0001
	.word	intr21-_start,1*8,0x8e00,0x0001
	.word	intr22-_start,1*8,0x8e00,0x0001
	.word	intr23-_start,1*8,0x8e00,0x0001
	.word	intr24-_start,1*8,0x8e00,0x0001
	.word	intr25-_start,1*8,0x8e00,0x0001
	.word	intr26-_start,1*8,0x8e00,0x0001
	.word	intr27-_start,1*8,0x8e00,0x0001
	.word	intr28-_start,1*8,0x8e00,0x0001
	.word	intr29-_start,1*8,0x8e00,0x0001
	.word	intr30-_start,1*8,0x8e00,0x0001
	.word	intr31-_start,1*8,0x8e00,0x0001
	.word	intr32-_start,1*8,0x8e00,0x0001
	.word	intr33-_start,1*8,0x8e00,0x0001
	.word	intr34-_start,1*8,0x8e00,0x0001
	.word	intr35-_start,1*8,0x8e00,0x0001
	.word	intr36-_start,1*8,0x8e00,0x0001
	.word	intr37-_start,1*8,0x8e00,0x0001
	.word	intr38-_start,1*8,0x8e00,0x0001
	.word	intr39-_start,1*8,0x8e00,0x0001
	.word	intr40-_start,1*8,0x8e00,0x0001
	.word	intr41-_start,1*8,0x8e00,0x0001
	.word	intr42-_start,1*8,0x8e00,0x0001
	.word	intr43-_start,1*8,0x8e00,0x0001
	.word	intr44-_start,1*8,0x8e00,0x0001
	.word	intr45-_start,1*8,0x8e00,0x0001
	.word	intr46-_start,1*8,0x8e00,0x0001
	.word	intr47-_start,1*8,0x8e00,0x0001
	.word	intr48-_start,1*8,0xee00,0x0001
	
# This is the initializer for the global interrupt table.
# It simply gives the size and location of the interrupt table
	
idt_init:
	.word	idt_init-idt
	.long	idt

# This is the initializer for an invalid interrupt table.
# Its only purpose is to assist with the reboot routine.
	
idt_invalid:
	.word	0
	.long	0
