add_library(io)

target_sources(io
	PRIVATE
    	io.c

	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
		.
		FILES
		io.h
)

target_link_libraries(io
  PRIVATE
    logging
)