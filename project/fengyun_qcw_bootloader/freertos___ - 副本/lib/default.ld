OUTPUT_FORMAT("elf32-sm32", "elf32-sm32", "elf32-sm32")
OUTPUT_ARCH(sm32)
_start = 0x100;
ENTRY(_start)
GROUP(-lor32 -lnewlib -lc -lm -lgcc -lmalloc)
MEMORY
{
  except : ORIGIN = 0x00000000, LENGTH = 0x1000
  ram : ORIGIN = (0x1000 + 0x0), LENGTH = 64M - (0x1000 + 0x0)
}
SECTIONS
{
  .reset :
  {
    *(.reset)
  } > except
  .codecs_header :
  {
    _codec_start_addr = .;
    codec_start_addr = .;
    KEEP(*(.codecs_header))
    . += 8K;
    codec_stack = .;
  } > ram
  .private_sec :
  {
    __private_start = .;
    KEEP(*(.private_sec))
    __private_end = .;
  } > ram
  .text :
  {
    _codec_end_addr = .;
    codec_end_addr = .;
    __text_start = .;
    *(.text .stub .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    *(.gnu.warning)
    __text_end = .;
  } > ram
  .data ALIGN(0x4) :
  {
    __CTOR_LIST__ = .;
    ___CTOR_LIST__ = .;
    LONG(-1)
    *(.ctors)
    LONG(0)
    __CTOR_END__ = .;
    __DTOR_LIST__ = .;
    ___DTOR_LIST__ = .;
    LONG(-1)
    *(.dtors)
    LONG(0)
    __DTOR_END__ = .;
    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.rodata1)
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    . = ALIGN(32 / 8);
  } > ram
  __bss_start = .;
  _bss_start = .;
  .bss ALIGN(0x4) :
  {
   *(.dynbss)
   *(.sbss .sbss.* .gnu.linkonce.b.*)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(32 / 8);
  } > ram
  _end = .;
  _bss_end = .;
  PROVIDE (end = .);
  _heap_start = .;
  . = (64M - 8K - 8K - 4);
  _heap_end = .;
  .stack ALIGN(4) (NOLOAD) :
    {
    __stack_start = .;
    . += 8K;
    _exception_stack = .;
    . += 8K;
    __stack_end = .;
    _stack = .;
    __end = .;
  } > ram
  /DISCARD/ : { *(.note.GNU-stack) }
}
