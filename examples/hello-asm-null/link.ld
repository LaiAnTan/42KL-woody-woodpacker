PHDRS {
  text_pt PT_LOAD FLAGS(5) FILEHDR;
  data_pt PT_LOAD FLAGS(6);
  note_pt PT_NOTE FLAGS(4);
}

SECTIONS {
	. = 0x401000;
	.text ALIGN(0x1)  : { *(.text) } : text_pt AT(0x1000)
	
	. = 0x401030;        /* Adjust address after .data */
	.note ALIGN(0x1) : {
		/* Complete ELF64 note: 12-byte header + name + desc + padding */
		LONG(4);      /* namesz = 4 */
		LONG(8);      /* descsz = 8 bytes */
		LONG(1);      /* type = 1 */
		BYTE(78); BYTE(79); BYTE(84); BYTE(69);
		LONG(0x12345678);  /* desc word 1 */
		LONG(0xdeadbeef);  /* desc word 2 */
		/* 8-byte alignment padding */
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		BYTE(0);
		} : note_pt


	. = 0x401080;
	.data ALIGN(0x1) : { *(.data) } : data_pt
}
