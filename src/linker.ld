 /* relevant material */
 /* https://wiki.osdev.org/Bare_Bones - linking the kernel section */
ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)
SECTIONS
{


    . = 0x100000;
    .text :
    {
        *(.multiboot)
        *(.text*)
        *(.rdata)
    }

    .data :
    {
        start_ctors = .;
        KEEP(*( .init_array));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.*)));
        end_ctors = .;
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }

    /DISCARD/ :
    {
        *(.fini_array*)
        *(.comment)

    }
}
