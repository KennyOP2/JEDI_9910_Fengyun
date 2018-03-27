ENTRY(codec_start)
OUTPUT_ARCH(sm32)
GROUP(-lc -lgcc)
SEARCH_DIR(.)
MEMORY
{
    PLUGIN_RAM : ORIGIN = (0x1000 + 0x0), LENGTH = 600K
}
SECTIONS
{
    .codecs_header :
    {
        _codec_start_addr = .;
        codec_start_addr = .;
        *(codec_crt0.o)
        KEEP(*(.codecs_header))
        . = ALIGN(4);
    } > PLUGIN_RAM
    .text ALIGN(4) :
    {
        *(.text*)
    } > PLUGIN_RAM
    .data ALIGN(4) :
    {
        *(.rodata*)
        *(.data*)
        . = ALIGN(4);
    } > PLUGIN_RAM
    /DISCARD/ :
    {
        *(.eh_frame)
    }
    .bss ALIGN(4) (NOLOAD):
    {
        codec_bss_start = .;
        _codec_bss_start = .;
        *(.bss* .sbss*)
        *(COMMON)
        . = ALIGN(0x4);
        _codec_bss_end = .;
        codec_bss_end = .;
        _codec_end_addr = .;
        codec_end_addr = .;
    } > PLUGIN_RAM
    .comment 0 :
    {
        KEEP(*(.comment))
    }
}
