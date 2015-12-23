# Copyright (C) 2015 The University of Notre Dame
# This software is distributed under the GNU General Public License.
# See the file LICENSE for details.

	.global syscall
syscall:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	movl	8(%ebp), %eax
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
	movl	20(%ebp), %edx
	movl	24(%ebp), %esi
	movl	28(%ebp), %edi
	int	$48
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	addl	$4,%esp
	leave
	ret
