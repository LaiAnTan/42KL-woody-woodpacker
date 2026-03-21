PHDRS {
  text_pt PT_LOAD FLAGS(5);    /* PF_R|PF_X = 4+1 = 5 */
  data_pt PT_LOAD FLAGS(6);    /* PF_R|PF_W = 4+2 = 6 */
  text2_pt PT_LOAD FLAGS(5);   /* PF_R|PF_X = 5 */
}


SECTIONS {
	. = 0x401000;
	.text : { *(.text) }: text_pt
	. = 0x401030;
	.data : { *(.data) }: data_pt
	. = 0x401040;
	.text2 : { *(.text2) }: text2_pt
}
