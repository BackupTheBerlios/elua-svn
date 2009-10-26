#include "stacks.h"

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

#define ARM_MODE_ABT     0x17
#define ARM_MODE_FIQ     0x11
#define ARM_MODE_IRQ     0x12
#define ARM_MODE_SVC     0x13

#define I_BIT            0x80
#define F_BIT            0x40


#define RAM_Base         0x40000000
#define RAM_Size         0x10000        // [TODO] make this 96k?
#define Top_Stack        (RAM_Base + RAM_Size)

//------------------------------------------------------------------------------
//         Startup routine
//------------------------------------------------------------------------------

            .align      4
            .arm
        
/* Exception vectors
 *******************/
            .section    .vectors, "a"

resetVector:
         ldr     pc, =resetHandler       /* Reset */
undefVector:
         b       undefVector             /* Undefined instruction */
swiVector:
         b       swiVector               /* Software interrupt */
prefetchAbortVector:
         b       prefetchAbortVector     /* Prefetch abort */
dataAbortVector:
         b       dataAbortVector         /* Data abort */
reservedVector:
         b       reservedVector          /* Reserved for future use */
irqVector:
         ldr     pc, [pc, #-0x0120]      /* Vector from VicVectAddr */
fiqVector:
         b       fiqVector               /* Fast interrupt */

//------------------------------------------------------------------------------
/// Initializes the chip and branches to the main() function.
//------------------------------------------------------------------------------
            .section    .text
            .global     entry
            .extern     main
            .extern     TargetResetInit

entry:
resetHandler:

/* Setup stacks for each mode */
        ldr     r0, =Top_Stack

        msr     CPSR_c, #ARM_MODE_IRQ|I_BIT|F_BIT
        mov     r13, r0
        sub     r0, r0, #STACK_SIZE_IRQ                  

        # Set up Supervisor Mode and set Supervisor Mode Stack (leave interrupts enabled)
        msr     CPSR_c, #ARM_MODE_SVC|F_BIT
        mov     r13, r0

/* Call external initialization code */
        bl      TargetResetInit

/* Perform low-level initialization of the chip using LowLevelInit() */
/* Initialize the relocate segment */

        ldr     r0, =_efixed
        ldr     r1, =_srelocate
        ldr     r2, =_erelocate
CopyROMtoRAM:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     CopyROMtoRAM

/* Clear the zero segment */
   	    ldr     r0, =_szero
        ldr     r1, =_ezero
        mov     r2, #0
ZeroBSS:
        cmp     r0, r1
        strcc   r2, [r0], #4
        bcc     ZeroBSS

/* Branch to main()
 ******************/
        ldr     r0, =main
        mov     lr, pc
        bx      r0

/* Loop indefinitely when program is finished */
forever:
        b       forever

        .end

