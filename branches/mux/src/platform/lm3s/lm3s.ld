MEMORY
{
    sram (W!RX) : ORIGIN = 0x20000000, LENGTH = 0x00010000
    flash (RX) : ORIGIN = 0x00000000, LENGTH = 0x00040000
}

SECTIONS
{  
    .text :
    {
        . = ALIGN(4);
        _text = .;
        PROVIDE(stext = .);
        KEEP(*(.isr_vector))
        KEEP(*(.init))
        *(.text .text.*)        
        *(.rodata .rodata.*)        
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        _etext = .;
        PROVIDE(etext = .);   
     		_fini = . ;
				*(.fini)

    } >flash

    .data : AT (_etext)
    {
        . = ALIGN(4);
        _data = .;
        *(.ramfunc .ramfunc.* .fastrun .fastrun.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
    } >sram
		
		.ARM.extab :
		{
		    *(.ARM.extab*)
		} >sram
		
		__exidx_start = .;
		.ARM.exidx :
		{
		    *(.ARM.exidx*)
		} >sram
		__exidx_end = .;
		
    .bss (NOLOAD) : {
        _bss = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);        
        _ebss = .;
    } >sram
    
    end = .;
}
