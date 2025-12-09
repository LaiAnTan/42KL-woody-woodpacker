add_library(types)

target_sources(types
	PRIVATE
	types.c

	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
		.
		FILES
		types.h
)
