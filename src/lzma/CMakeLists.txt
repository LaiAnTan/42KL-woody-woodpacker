add_library(lzma)

target_sources(lzma
	PRIVATE
		Alloc.c
		CpuArch.c
		LzFind.c
		LzFindMt.c
		LzFindOpt.c
		LzmaDec.c
		LzmaEnc.c
		Threads.c

	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			.
		FILES
			7zTypes.h
			Alloc.h
			Compiler.h
			CpuArch.h
			LzFind.h
			LzFindMt.h
			LzHash.h
			LzmaDec.h
			LzmaEnc.h
			Precomp.h
			Threads.h

)