# Copyright (C) 2015 The University of Notre Dame
# This software is distributed under the GNU General Public License.
# See the file LICENSE for details.

# This is the raw bootblock code, a 512-byte chunk of assembly
# found on the first sector of the boot disk.  The BIOS is responsible
# for starting the machine, loading this sector into memory,
# and then transferring control here.  The bootblock must call
# to the BIOS to load the remaining sectors containing the
# kernel code, and then jump there.

# Constants describing our basic memory layout are in this
# header file, which is shared between C and assembly modules:
	
#include "memorylayout.h"
	
# When we receive control from the BIOS, the following are set:
# %dl - the device number this block was loaded from
# %es:%si - the partition table entry we were loaded from

# To set the code segment appropriately, the first thing we
# do is a long jump to _start2, which sets cs=BOOTBLOCK_SEGMENT
	
.code16
.text
.global _start
_start:
	ljmp	$BOOTBLOCK_SEGMENT,$_start2

# Now we begin setting up the execution environment
# for loading the rest of the kernel.

_start2:	
	sti				# disable interrupts
	cld				# clear the direction flag
	mov	%cs, %ax		# set all segments to code
	mov	%ax, %ds
	mov	%ax, %es
	mov	$INTERRUPT_STACK_SEGMENT, %ax     # set up the stack
	mov	%ax, %ss
	mov	$INTERRUPT_STACK_OFFSET, %sp

	mov	%dl, (disk_number)	# save the disk number
	mov	partition_status,%di	# set the partition table as dest
	mov	$12, %cx		# copy 12 bytes from si to di
	rep	movsb
		
	mov	$(loadmsg),%si		# print initial message
        call	bios_putstring

	mov	$0,%ah			# reset the disk system
	int	$0x13

	mov	$0x08, %ah		# get the drive geometry
	int	$0x13
	and	$0x3f, %cl		# mask off high tracks
	mov	%cl, (disk_sectors)
	mov	%ch, (disk_cylinders)
	mov	%dh, (disk_heads)
	
	mov	$KERNEL_SEGMENT,%ax	# load happens at es:bx
	mov	%ax, %es		# which we set to
	mov	$KERNEL_OFFSET,%bx	# KERNEL_SEGMENT:KERNEL_OFFSET

					# disk parameters:
	mov	(disk_number), %dl	#	device
	mov	$0,%ch			#	cylinder 0
	mov	$0,%dh			#	head 0
	mov	$2,%cl			#	sector 2

loadsector:
	mov	$1,%al			# load 1 sector
	mov	$0x02, %ah		# load command
	int	$0x13			# execute load
	
	mov	$'.', %al		# display a dot
	call	bios_putchar		# for each sector loaded
	
	mov	(sectors_left),%ax	# how many sectors left?
	cmp	$0xffff, %ax		# has it been initialized?
	jne	gotsectors		# yes - use the value
	mov	%es:(KERNEL_SIZE_OFFSET),%eax	# no - get size of kernel
	shr	$9, %eax		# convert into blocks
	inc	%eax			# add one for good measure
	
gotsectors:		
	dec	%ax			# remove one block
	mov	%ax,(sectors_left)	# store the value
	cmp	$0, %ax			# are we done?
	je	loaddone		# yes - jump to bottom
			
checksegment:
	add	$512,%bx		# move data pointer by 512 bytes
	cmp	$0, %bx			# did we reach segment end?
	jnz	nextsector		# no - find next sector
	mov	%es, %ax		# yes - retrieve seg register
	add	$0x1000, %ax		# move to next 64k block
	mov	%ax, %es		# store segment register

nextsector:
	inc	%cl			# advance by one sector
	mov	(disk_sectors),%al	# what is the maximum sector?
	cmp	%al, %cl		# is this the last sector?
	jle	loadsector		# no - load the next sector
	mov	$1,%cl			# yes - go to sector zero..

	inc	%dh			# advance to next head
	mov	(disk_heads), %al	# what is the maximum head?
	cmp	%al, %dh		# is this the last head?
	jle	loadsector		# no - read the next sector
	mov	$0,%dh			# yes - go to head zero

	inc	%ch			# advance to next cylinder
	mov	(disk_cylinders), %al	# what is the maximum cylinder?
	cmp	%al, %ch		# is this the last cylinder?
	jle	loadsector		# no - read the next sector
					# yes - fall through here
loaddone:
	mov	$0,%ah			# reset the disk system
	int	$0x13
	
	mov	$(bootmsg),%si		# print boot message
        call	bios_putstring
	
	mov	$KERNEL_SEGMENT, %ax    # jump to the kernel code
	mov	%ax, %ds
	ljmp	$KERNEL_SEGMENT, $KERNEL_OFFSET
	
bios_putstring:				# routine to print an entire string
	mov	(%si), %al
	cmp	$0, %al
	jz	bios_putstring_done
        call	bios_putchar
	inc	%si
	jmp	bios_putstring
bios_putstring_done:
        ret

bios_putchar:				# routine to print a single char
	push	%ax
	push	%bx
        mov	$14,%ah
        mov	$1,%bl
        int	$0x10
	pop	%bx
	pop	%ax
	ret

loadmsg:
        .asciz "bootblock: loading kernel...\r\n"
bootmsg:
	.asciz "\r\nbootblock: booting kernel...\r\n"

disk_number:
	.byte 0
disk_cylinders:
	.byte 0
disk_heads:
	.byte 0
disk_sectors:
	.byte 0
sectors_left:	
	.word 0xffff

partition_status:
	.byte 0
partition_start_chs:	
	.byte 0
	.byte 0
	.byte 0
partition_type:	
	.byte 0
partition_stop_chs:	
	.byte 0
	.byte 0
	.byte 0
partition_start_lba:
	.long 0
partition_length:
	.long 0
				
# A bootblock must have 0xaa55 in its two final bytes.
# The .org directive forces this data to that point.

.org 510
bootflag:
	.word 0xaa55
