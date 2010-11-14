#-------------------------------------------------------------------
# This file is part of the CMake build system for eqOgre
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )
install( DIRECTORY ${VL_DATA_PATH}/ogre_project DESTINATION ./ )
install( FILES ${VL_DATA_PATH}/env.env DESTINATION ./ )

# Copy start script
# Set variables for start scripts
set( PROGRAM_NAME spike_listening_client )
set( DEMO_DATA_PATH ./ )

set( TEMP_DATA_DIR ${vl_BINARY_DIR}/temp_data )

# Because CMake runs all commands here automatically when the file is included
# We need to create a temp dir for the config files.
# When make install is run the files are copied from temp_dir to install dir
if( WIN32 )
	configure_file( ${VL_DATA_PATH}/start.bat
		${TEMP_DATA_DIR}/start.bat )
else()
	configure_file( ${VL_DATA_PATH}/start.sh
		${TEMP_DATA_DIR}/start.sh )
endif()

# Install the configs from Temp dir
if( WIN32 )
	install( FILES ${TEMP_DATA_DIR}/start.bat DESTINATION ./ )
else()
	install( FILES ${TEMP_DATA_DIR}/start.sh DESTINATION ./ )
endif()

install( FILES
	${VL_DATA_PATH}/The_Dummy_Song.ogg
	DESTINATION ./ )
