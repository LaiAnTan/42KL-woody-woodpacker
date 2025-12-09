#include <stdio.h>

#include "logging.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"

int validate_args(int argc, char const *argv[])
{
	// TODO: add more inputs for parameterized key bonus
	if (argc != 2)
	{
		printf("usage: woody <file_name> <aes-key-or-whatever>\n");
		return 1;
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	if (validate_args(argc, argv))
		return 1;

	// CLzmaEncProps props;
	// LzmaEncProps_Init(&props);

	return 0;
}
