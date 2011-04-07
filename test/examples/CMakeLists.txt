if( WIN32 )
	add_executable( nv_swap_sync WIN32 nv_swap_sync.cpp )

	target_link_libraries( nv_swap_sync ${OPENGL_LIBRARIES} )
endif()