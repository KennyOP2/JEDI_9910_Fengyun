/*
 * SMedia Tech. Corp. 2008
 * default link script
 *
 * defined symbol: MEMORY_SIZE (= 8M, 16M, or 32M)
 *                 CODEC_START_ADDR
 *
 * Notes: it is not a standard link script, must use gcc preprocessor
 *        to generate the standard link script.
 */

#include "codecs_defs.h"

// sm32 is predefined by gcc compiler, must undefine it.
#undef  sm32

// define stack size
#define EXCEPTION_STACK_SIZE    8K
#define STACK_SIZE              8K

// begin of link script
OUTPUT_FORMAT("elf32-sm32", "elf32-sm32", "elf32-sm32")
OUTPUT_ARCH(sm32)
_start = 0x100;
ENTRY(_start)

#if defined(CONFIG_HAVE_USBD)
GROUP(-lor32 -lnewlib -lc -lm -lgcc -lstdc++ -lmalloc)
#else
GROUP(-lor32 -lnewlib -lc -lm -lgcc -lmalloc)
#endif

MEMORY
{
  except : ORIGIN = 0x00000000, LENGTH = 0x1000
//  msgq   : ORIGIN = 0x00001000, LENGTH = 0x0030
  ram    : ORIGIN = CODEC_START_ADDR, LENGTH = MEMORY_SIZE - CODEC_START_ADDR
}

SECTIONS
{
  .reset :
  {
    *(.reset)
  } > except

//#if defined(HAVE_NANDBOOT)
//  .boot :
//  {
//    *(.boot_data)
//    . = ALIGN(32 / 8);
//    *(.boot)
//    PROVIDE (__boot_end = .);
//  } > except
//#endif // defined(HAVE_NANDBOOT)

//  .common_var :
//  {
//    *(.common_var)
//  } > msgq

  .codecs_header :
  {
    _codec_start_addr = .;
    codec_start_addr  = .;
    KEEP(*(.codecs_header))

    // codec stack for audio codec on processor 2
#ifndef BUILDING_BOOTLOADER
    . += 8K;
#endif
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
    codec_end_addr  = .;

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
    /* In 3.4.4, libgcc2.c actually required -1 here, rather than the count
       , as in ld documentation
    */
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
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.  */
   . = ALIGN(32 / 8);
  } > ram
  _end = .;
  _bss_end = .;
  PROVIDE (end = .);

  _heap_start = .;
  . = (MEMORY_SIZE - STACK_SIZE - EXCEPTION_STACK_SIZE - 4);
  _heap_end   = .;

  .stack ALIGN(4) (NOLOAD) :
    {
    __stack_start = .;
    . += STACK_SIZE;
    _exception_stack = .;

    . += EXCEPTION_STACK_SIZE;
    __stack_end = .;
    _stack      = .;
    __end       = .;
  } > ram

  /DISCARD/ : { *(.note.GNU-stack) }
}
