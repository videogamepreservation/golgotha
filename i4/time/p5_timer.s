#include <linux/sys.h>
#include <linux/linkage.h>

.data
timer_hi:	.long	0
timer_lo:	.long 0
.text

#define rdtsc	\
.byte	0x0f;	\
.byte		0x31

.globl i4_pentium_start_clock
i4_pentium_start_clock:
	rdtsc
	movl	%edx, timer_hi
	movl	%eax, timer_lo
	ret

.globl i4_pentium_end_clock
i4_pentium_end_clock:
	rdtsc
	movl	4(%esp), %ecx
	subl	timer_lo, %eax
	sbbl	timer_hi, %edx
	movl	%eax, 4(%ecx)
	movl	%edx, (%ecx)
	ret

