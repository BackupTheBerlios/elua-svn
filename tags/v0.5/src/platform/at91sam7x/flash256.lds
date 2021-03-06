/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 * ----------------------------------------------------------------------------
 * Copyright (c) 2006, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaiimer below.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation and/or
 * other materials provided with the distribution.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------------
 *      Linker script for running in internal Flash on the AT91SAM7X256.
 *----------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(entry)

MEMORY
{
    sram (W!RX) : ORIGIN = 0x200000, LENGTH = 0x10000
    flash (RX) : ORIGIN = 0x100000, LENGTH = 0x40000
}

SECTIONS
{  
    .fixed :
    {
        . = ALIGN(4);
        _sfixed = .;
        KEEP(*(.vectors))
        KEEP(*(.init))
        *(.text .text.*)        
        *(.rodata .rodata.*)        
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        _efixed = .;
        PROVIDE(etext = .);        
    } >flash

    .relocate : AT (_efixed)
    {
        . = ALIGN(4);
        _srelocate = .;
        *(.ramfunc .ramfunc.* .fastrun .fastrun.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _erelocate = .;
    } >sram

    .bss (NOLOAD) : {
        _szero = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        _ezero = .;
    } >sram
    
    end = .;
    _sstack = 0x210000;
}
