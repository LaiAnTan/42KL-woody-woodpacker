#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"

// gcc *.c -I .

void printLzmaStatus(ELzmaStatus status) {
    switch (status) {
        case LZMA_STATUS_NOT_SPECIFIED:
            printf("Status: NOT_SPECIFIED\n");
            break;
        case LZMA_STATUS_FINISHED_WITH_MARK:
            printf("Status: FINISHED_WITH_MARK\n");
            break;
        case LZMA_STATUS_NOT_FINISHED:
            printf("Status: NOT_FINISHED\n");
            break;
        case LZMA_STATUS_NEEDS_MORE_INPUT:
            printf("Status: NEEDS_MORE_INPUT\n");
            break;
        case LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK:
            printf("Status: MAYBE_FINISHED_WITHOUT_MARK\n");
            break;
        default:
            printf("Status: UNKNOWN\n");
            break;
    }
}

int	main(int argc, char **argv)
{
	char *og = "hello world!!!!";

    CLzmaEncProps props;
	LzmaEncProps_Init(&props);
		
	SizeT encoded_len = 50;
	Byte *encoded = calloc(encoded_len, 1);
	// memset(encoded, 69, 20);

	// NOTE: this needs to be shared by both sides
	Byte prop_data[LZMA_PROPS_SIZE];
    SizeT prop_size = LZMA_PROPS_SIZE;


	printf("To encode: \n");
	for (size_t i = 0; i < strlen(og); i++)
		printf("%d ", og[i]);
	printf("\n");

	SRes res = LzmaEncode(encoded, &encoded_len, (Byte *) og, strlen(og), &props, prop_data, &prop_size, 1, 0, &g_Alloc, &g_Alloc);
	
	
	if (res == SZ_OK) {
		printf("Encoding OK, size %d\n", encoded_len);

		// print encoded after after encoding
		printf("Encode result: ");
		for (size_t i = 0; i < encoded_len; i++)
			printf("%d ", encoded[i]);
		printf("\n");

		SizeT decoded_len = 50;
		Byte *decoded = calloc(decoded_len, 1);
		// memset(decoded, 69, 20);

		Byte props_decoded[LZMA_PROPS_SIZE];
   		SizeT dec_prop_size = LZMA_PROPS_SIZE;

		// Byte *props_decoded = calloc(5, 1);
		// SizeT dec_prop_size = LZMA_PROPS_SIZE;

		ELzmaStatus decode_status;

		res = LzmaDecode(decoded, &decoded_len,
			encoded, &encoded_len,
			props_decoded, dec_prop_size,
			LZMA_FINISH_END,
			&decode_status,
			&g_Alloc);




		printf("Decode result: ");
		for (size_t i = 0; i < decoded_len; i++)
			printf("%d ", decoded[i]);
		printf("\n");
		
		printLzmaStatus(decode_status);
		printf("Reulst %d\n", res);
		return res;
	}
	
	return res;
}


