/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020/12/12     bernard      The first version
 */

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x20000;
__min_heap_size = 0x200;


MEMORY
{
   DRAM : ORIGIN = 0x80050000, LENGTH = 0x3F60000
}

PROVIDE (__heap_end  = ORIGIN(DRAM) + LENGTH(DRAM));

REGION_ALIAS("REGION_TEXT",    DRAM);
REGION_ALIAS("REGION_RODATA",  DRAM);
REGION_ALIAS("REGION_DATA",    DRAM);
REGION_ALIAS("REGION_BSS",     DRAM);

ENTRY(_start)
SECTIONS
{
.text : {
   . = ALIGN(8);
   KEEP(*start.o(*.text))
   KEEP(*start.o(*.vectors))
   KEEP(*_gcc.o)
  *(.text)
  *(.text*)
  *(.text.*)
  *(.gnu.warning)
  *(.stub)
  *(.gnu.linkonce.t*)
  *(.glue_7t)
  *(.glue_7)
  *(.jcr)
  *(.gnu.linkonce.armextab.*)
 /* section information for finsh shell */
   . = ALIGN(8);
   __fsymtab_start = .;
   KEEP(*(FSymTab))
   __fsymtab_end = .;
   . = ALIGN(8);
   __vsymtab_start = .;
   KEEP(*(VSymTab))
   __vsymtab_end = .;
   . = ALIGN(8);

   /* section information for initial. */
   . = ALIGN(8);
   __rt_init_start = .;
   KEEP(*(SORT(.rti_fn*)))
   __rt_init_end = .;
   . = ALIGN(8);

   __rt_utest_tc_tab_start = .;
   KEEP(*(UtestTcTab))
   __rt_utest_tc_tab_end = .;

   . = ALIGN(8);
   __etext = .;
} > REGION_TEXT

.rodata : {
   . = ALIGN(8);
   __rodata_start = .;
   *(.rdata)
   *(.rdata*)
   *(.rdata1)
   *(.rdata.*)
   *(.rodata)
   *(.rodata1)
   *(.rodata1.*)
   *(.rodata*)
   *(.rodata.*)
   *(.rodata.str1.4)
   *(.srodata*)
   *(.gcc_except_table)
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
   __rodata_end = .;
} > REGION_RODATA

.data : {
   . = ALIGN(8);
   _data = .;
   *(.data)
   *(.data.*)
   *(.data1)
   *(.data1.*)
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.d.*)
   *(.jcr)
   *(.got)
   *(.got1)
   *(.got2)
   *(.got.plt)
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
   __fixup_start = .;
   *(.fixup)
   __fixup_end = .;
   *(.eh_frame)
   __eh_framehdr_start = .;
   *(.eh_framehdr)
   __eh_framehdr_end = .;
   _edata = .;
} > REGION_DATA

.bss (NOLOAD): {
   . = ALIGN(0x8) ;
   _bss = . ;
   *(.dynsbss)
   *(.sbss)
   *(.sbss.*)
   *(.scommon)
   *(.dynbss)
   *(.bss)
   *(.bss.*)
   *(COMMON)
   . = ALIGN(0x8) ;
   _ebss = . ;
} > REGION_BSS

/*_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );*/
_data_lma = LOADADDR(.data);

/* Generate Stack and Heap definitions */
.stack (NOLOAD) : {
   . = ALIGN(8);
   _stack_end_end = .;
   . += _STACK_SIZE;
   _stack_top = .;
  __rt_rvstack = .; 
} > DRAM

.heap (NOLOAD) : {
   . = ALIGN(0x8) ;
   _heap =  .;
   HeapBase = .;
  . += __min_heap_size;
   HeapLimit = .;
} > DRAM

HeapLimit = __heap_end;
    _end = .;

   /* Stabs debugging sections.  */
   .stab          0 : { *(.stab) }
   .stabstr       0 : { *(.stabstr) }
   .stab.excl     0 : { *(.stab.excl) }
   .stab.exclstr  0 : { *(.stab.exclstr) }
   .stab.index    0 : { *(.stab.index) }
   .stab.indexstr 0 : { *(.stab.indexstr) }
   .comment       0 : { *(.comment) }
   /* DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to the beginning
   * of the section so we begin them at 0.  */
   /* DWARF 1 */
   .debug          0 : { *(.debug) }
   .line           0 : { *(.line) }
   /* GNU DWARF 1 extensions */
   .debug_srcinfo  0 : { *(.debug_srcinfo) }
   .debug_sfnames  0 : { *(.debug_sfnames) }
   /* DWARF 1.1 and DWARF 2 */
   .debug_aranges  0 : { *(.debug_aranges) }
   .debug_pubnames 0 : { *(.debug_pubnames) }
   /* DWARF 2 */
   .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
   .debug_abbrev   0 : { *(.debug_abbrev) }
   .debug_line     0 : { *(.debug_line) }
   .debug_frame    0 : { *(.debug_frame) }
   .debug_str      0 : { *(.debug_str) }
   .debug_loc      0 : { *(.debug_loc) }
   .debug_macinfo  0 : { *(.debug_macinfo) }
   /* SGI/MIPS DWARF 2 extensions */
   .debug_weaknames 0 : { *(.debug_weaknames) }
   .debug_funcnames 0 : { *(.debug_funcnames) }
   .debug_typenames 0 : { *(.debug_typenames) }
   .debug_varnames  0 : { *(.debug_varnames) }
}
