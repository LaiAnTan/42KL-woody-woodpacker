PHDRS {
  text_pt PT_LOAD FLAGS(5) FILEHDR;
  data_pt PT_LOAD FLAGS(6);
  note_pt PT_SHLIB FLAGS(4);
}

SECTIONS {
	. = 0x401000;
	.text ALIGN(0x1)  : { *(.text) } : text_pt
	
	. = 0x401030;        /* Adjust address after .data */
	.note ALIGN(0x1) : {
		
		} : note_pt


	. = 0x401080;
	.data ALIGN(0x1) : { *(.data) } : data_pt
}
