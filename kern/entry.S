/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
//#include <kern/cpu/cpu.h>


# Shift Right Logical
#define SRL(val, shamt)		(((val) >> (shamt)) & ~(-1 << (32 - (shamt))))


###################################################################
# The kernel (this code) is linked at address ~(KERNEL_BASE + 1 Meg),
# but the bootloader load it at address	~1 Meg.
#
# RELOC(x) maps a symbol x from its link address to its actual
# location in physical memory (its load address).
###################################################################

#define	RELOC(x) ((x) - KERNEL_BASE)


.set CODE_SEL,0x8		# index of code seg within mygdt
.set DATA_SEL,0x10		# index of data seg within mygdt

#define MULTIBOOT_PAGE_ALIGN  (1<<0)
#define MULTIBOOT_MEMORY_INFO (1<<1)
#define MULTIBOOT_HEADER_MAGIC (0x1BADB002)
#define MULTIBOOT_HEADER_FLAGS (MULTIBOOT_MEMORY_INFO | MULTIBOOT_PAGE_ALIGN)
#define CHECKSUM (-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS))

###################################################################
# entry point
###################################################################

.text

# The Multiboot header
.align 4
.long MULTIBOOT_HEADER_MAGIC
.long MULTIBOOT_HEADER_FLAGS
.long CHECKSUM

.globl		start_of_kernel
start_of_kernel:
	movw	$0x1234,0x472			# warm boot

	# Establish our own GDT in place of the boot loader's temporary GDT.
	lgdt	RELOC(mygdtdesc)		# load descriptor table

	# Immediately reload all segment registers (including CS!)
	# with segment selectors from the new GDT.
	movl	$DATA_SEL, %eax			# Data segment selector
	movw	%ax,%ds				# -> DS: Data Segment
	movw	%ax,%es				# -> ES: Extra Segment
	movw	%ax,%ss				# -> SS: Stack Segment
	ljmp	$CODE_SEL,$relocated		# reload CS by jumping
relocated:

	# Clear the frame pointer register (EBP)
	# so that once we get into debugging C code,
	# stack backtraces will be terminated properly.
	movl	$0x0,%ebp			# nuke frame pointer

    # Leave a few words on the stack for the user trap frame
	#2024: this line is changed since the trapframe is move to the user kernel stack of each process
	#movl	$(ptr_stack_top-SIZEOF_STRUCT_TRAPFRAME),%esp
	movl	$(ptr_stack_top),%esp

	# now to C code
	call	FOS_initialize

	# Should never get here, but in case we do, just spin.
spin:	jmp	spin


###################################################################
# See <inc/memlayout.h> for a complete description of these two symbols.
###################################################################
.data
	.globl	vpt
	.set	vpt, VPT
	.globl	vpd
	.set	vpd, (VPT + SRL(VPT, 10))


###################################################################
# boot stack
###################################################################
	.p2align	PGSHIFT		# force page alignment
	.globl		ptr_stack_bottom
ptr_stack_bottom:
	.space		NCPUS*KERNEL_STACK_SIZE
	.globl		ptr_stack_top
ptr_stack_top:

###################################################################
# setup the GDT
###################################################################
	.p2align	2		# force 4 byte alignment
mygdt:
	SEG_NULL				# null seg
	SEG(STA_X|STA_R, -KERNEL_BASE, 0xffffffff)	# code seg
	SEG(STA_W, -KERNEL_BASE, 0xffffffff)		# data seg
mygdtdesc:
	.word	0x17				# sizeof(mygdt) - 1
	.long	RELOC(mygdt)		# address mygdt

