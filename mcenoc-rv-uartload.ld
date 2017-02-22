MEMORY {
    /* Specify the ROM even though we don't touch it in UART loaded programs */
    rom (rx) : ORIGIN = 0x00000000, LENGTH = 2K
    /* RAM starts 4 after the true start, so that we do not trash the interrupt
        vector pointer */
    ram : ORIGIN = 0x00008000, LENGTH = 32K
}

reset_addr = 0x4;
/* Put the IRQ vector at the beginning of RAM */
irq_vec_ptr = ORIGIN(ram);
stack_size = 4K;
irq_stack_size = 1K;
ram_end = ORIGIN(ram) + LENGTH(ram) - 4;
stack_bot = stack_top - stack_size;
stack_top = ram_end - irq_stack_size;
irq_stack_top = ram_end;


STARTUP(crt0.o);
ENTRY(_start);

SECTIONS {
    .text : {
        crt0.o(*)
        *(.text)
        *(.text*)
    } > ram
    .rodata : {
        *(.rodata)
        *(.rodata*)
        *(.comment)
    } > ram
    .data : {
        *(.data)
        *(.rwdata)
        *(.rwdata*)
    } > ram
    .bss : {
        bss_start = .;
        *(.bss)
        *(.sbss)
        *(COMMON)
        . = ALIGN(4);
        bss_end = .;
        ASSERT(. < stack_bot, "Error: Program invades stack"); 
    } > ram
}
