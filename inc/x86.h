#ifndef FOS_INC_X86_H
#define FOS_INC_X86_H

#include <inc/types.h>
#include <inc/mmu.h>

static __inline void breakpoint(void) __attribute__((always_inline));
static __inline uint8 inb(int port) __attribute__((always_inline));
static __inline void insb(int port, void *addr, int cnt) __attribute__((always_inline));
static __inline uint16 inw(int port) __attribute__((always_inline));
static __inline void insw(int port, void *addr, int cnt) __attribute__((always_inline));
static __inline uint32 inl(int port) __attribute__((always_inline));
static __inline void insl(int port, void *addr, int cnt) __attribute__((always_inline));
static __inline void outb(int port, uint8 data) __attribute__((always_inline));
static __inline void outsb(int port, const void *addr, int cnt) __attribute__((always_inline));
static __inline void outw(int port, uint16 data) __attribute__((always_inline));
static __inline void outsw(int port, const void *addr, int cnt) __attribute__((always_inline));
static __inline void outsl(int port, const void *addr, int cnt) __attribute__((always_inline));
static __inline void outl(int port, uint32 data) __attribute__((always_inline));
static __inline void invlpg(void *addr) __attribute__((always_inline));
//static __inline void lidt(void *p) __attribute__((always_inline));
static __inline void lldt(uint16 sel) __attribute__((always_inline));
static __inline void ltr(uint16 sel) __attribute__((always_inline));
static __inline void lcr0(uint32 val) __attribute__((always_inline));

//Ahmed 2010
static __inline void lcr2(uint32 val) __attribute__((always_inline));

//2024
static __inline void cli() __attribute__((always_inline));
static __inline void sti() __attribute__((always_inline));
static __inline uint32 xchg(volatile uint32 *addr, uint32 newval) __attribute__((always_inline));
static __inline void lgdt(struct Segdesc *p, int size) __attribute__((always_inline));
static __inline void lidt(struct Gatedesc *p, int size) __attribute__((always_inline));
//****************
static __inline uint32 rcr0(void) __attribute__((always_inline));
static __inline uint32 rcr2(void) __attribute__((always_inline));
static __inline void lcr3(uint32 val) __attribute__((always_inline));
static __inline uint32 rcr3(void) __attribute__((always_inline));
static __inline void lcr4(uint32 val) __attribute__((always_inline));
static __inline uint32 rcr4(void) __attribute__((always_inline));
static __inline void tlbflush(void) __attribute__((always_inline));
static __inline uint32 read_eflags(void) __attribute__((always_inline));
static __inline void write_eflags(uint32 eflags) __attribute__((always_inline));
static __inline uint32 read_ebp(void) __attribute__((always_inline));
static __inline uint32 read_esp(void) __attribute__((always_inline));
static __inline void write_esp(uint32 esp) __attribute__((always_inline));
static __inline void write_ebp(uint32 ebp) __attribute__((always_inline));
static __inline void cpuid(uint32 info, uint32 *eaxp, uint32 *ebxp, uint32 *ecxp, uint32 *edxp);
static __inline uint64 read_tsc(void) __attribute__((always_inline));

static __inline void
breakpoint(void)
{
	__asm __volatile("int3");
}

static __inline uint8
inb(int port)
{
	uint8 data;
	__asm __volatile("inb %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insb(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsb"			:
			 "=D" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory", "cc");
}

static __inline uint16
inw(int port)
{
	uint16 data;
	__asm __volatile("inw %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insw(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsw"			:
			 "=D" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory", "cc");
}

static __inline uint32
inl(int port)
{
	uint32 data;
	__asm __volatile("inl %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insl(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsl"			:
			 "=D" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory", "cc");
}

static __inline void
outb(int port, uint8 data)
{
	__asm __volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsb(int port, const void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\toutsb"		:
			 "=S" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "cc");
}

static __inline void
outw(int port, uint16 data)
{
	__asm __volatile("outw %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsw(int port, const void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\toutsw"		:
			 "=S" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "cc");
}

static __inline void
outsl(int port, const void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\toutsl"		:
			 "=S" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "cc");
}

static __inline void
outl(int port, uint32 data)
{
	__asm __volatile("outl %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
invlpg(void *addr)
{
	__asm __volatile("invlpg (%0)" : : "r" (addr) : "memory");
}

//static __inline void
//lidt(void *p)
//{
//	__asm __volatile("lidt (%0)" : : "r" (p));
//}

static __inline void
lldt(uint16 sel)
{
	__asm __volatile("lldt %0" : : "r" (sel));
}

static __inline void
ltr(uint16 sel)
{
	__asm __volatile("ltr %0" : : "r" (sel));
}

static __inline void
lcr0(uint32 val)
{
	__asm __volatile("movl %0,%%cr0" : : "r" (val));
}

static __inline uint32
rcr0(void)
{
	uint32 val;
	__asm __volatile("movl %%cr0,%0" : "=r" (val));
	return val;
}

//Ahmed 2010:
static __inline void
lcr2(uint32 val)
{
	__asm __volatile("movl %0,%%cr2" : : "r" (val));
}




static __inline uint32
rcr2(void)
{
	uint32 val;
	__asm __volatile("movl %%cr2,%0" : "=r" (val));
	return val;
}

static __inline void
lcr3(uint32 val)
{
	__asm __volatile("movl %0,%%cr3" : : "r" (val));
}

static __inline uint32
rcr3(void)
{
	uint32 val;
	__asm __volatile("movl %%cr3,%0" : "=r" (val));
	return val;
}

static __inline void
lcr4(uint32 val)
{
	__asm __volatile("movl %0,%%cr4" : : "r" (val));
}

static __inline uint32
rcr4(void)
{
	uint32 cr4;
	__asm __volatile("movl %%cr4,%0" : "=r" (cr4));
	return cr4;
}

static __inline void
tlbflush(void)
{
	uint32 cr3;
	__asm __volatile("movl %%cr3,%0" : "=r" (cr3));
	__asm __volatile("movl %0,%%cr3" : : "r" (cr3));
}

static __inline uint32
read_eflags(void)
{
        uint32 eflags;
        __asm __volatile("pushfl; popl %0" : "=r" (eflags));
        return eflags;
}

static __inline void
write_eflags(uint32 eflags)
{
        __asm __volatile("pushl %0; popfl" : : "r" (eflags));
}

static __inline uint32
read_ebp(void)
{
        uint32 ebp;
        __asm __volatile("movl %%ebp,%0" : "=r" (ebp));
        return ebp;
}

static __inline uint32
read_esp(void)
{
        uint32 esp;
        __asm __volatile("movl %%esp,%0" : "=r" (esp));
        return esp;
}

static __inline void
write_esp(uint32 esp)
{
	__asm __volatile("movl %0,%%esp" : : "r" (esp) );
}

static __inline void
write_ebp(uint32 ebp)
{
	__asm __volatile("movl %0,%%ebp" : : "r" (ebp) );
}

static __inline void
cpuid(uint32 info, uint32 *eaxp, uint32 *ebxp, uint32 *ecxp, uint32 *edxp)
{
	uint32 eax, ebx, ecx, edx;
	asm volatile("cpuid"
		: "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
		: "a" (info));
	if (eaxp)
		*eaxp = eax;
	if (ebxp)
		*ebxp = ebx;
	if (ecxp)
		*ecxp = ecx;
	if (edxp)
		*edxp = edx;
}

static __inline uint64
read_tsc(void)
{
        uint64 tsc;
        __asm __volatile("rdtsc" : "=A" (tsc));
        return tsc;
}

/*2024: newly added functions from xv6-x86 code el7 :)
 * https://github.com/mit-pdos/xv6-public
 */
//clear interrupt flag
static __inline void
cli(void)
{
	__asm __volatile("cli");
}

//set interrupt flag
static __inline void
sti(void)
{
	__asm __volatile("sti");
}

//atomic xchange
//Example: xchg(&(globalIntVar), 1);
static __inline uint32
xchg(volatile uint32 *addr, uint32 newval)
{
  uint32 result;
  // The + in "+m" denotes a read-modify-write operand.
  __asm __volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

//load GDT register
static __inline void
lgdt(struct Segdesc *p, int size)
{
  volatile unsigned short pd[3];

  pd[0] = size-1;
  pd[1] = (uint32)p;
  pd[2] = (uint32)p >> 16;

  __asm __volatile("lgdt (%0)" : : "r" (pd));
}

//load IDT register
static __inline void lidt(struct Gatedesc *p, int size)
{
  volatile unsigned short pd[3];

  pd[0] = size-1;
  pd[1] = (uint32)p;
  pd[2] = (uint32)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}
#endif /* !FOS_INC_X86_H */
