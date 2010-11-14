
set(CPACK_PACKAGE_INSTALL_DIRECTORY "eqOgre")
set(CPACK_PACKAGE_NAME "eqOgre")
set(CPACK_PACKAGE_CONTACT "Joonatan Kuosa <joonatan.kuosa@tut.fi>")

if( WIN32 AND NOT UNIX )
	# TODO these should be set as options
	set(CPACK_GENERATOR "NSIS;ZIP;")
elseif()
	# TODO these should be set as options
	set(CPACK_GENERATOR "TBZ2;TGZ;")
endif()

include(CPack)