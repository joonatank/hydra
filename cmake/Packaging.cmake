#-------------------------------------------------------------------
# This file is part of the CMake build system for eqOgre
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

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