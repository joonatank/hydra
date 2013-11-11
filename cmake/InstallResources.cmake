#-------------------------------------------------------------------
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set( DATA_PATH ${PROJECT_SOURCE_DIR}/data )

install( DIRECTORY ${DATA_PATH}/ogre_project DESTINATION data CONFIGURATIONS Release)
install( DIRECTORY ${DATA_PATH}/global DESTINATION data CONFIGURATIONS Release )
install( DIRECTORY ${DATA_PATH}/tracking DESTINATION data CONFIGURATIONS Release )
install( DIRECTORY ${DATA_PATH}/generated_physics_project DESTINATION data CONFIGURATIONS Release )
install( DIRECTORY ${DATA_PATH}/perapora_project DESTINATION data CONFIGURATIONS Release )
install( DIRECTORY ${DATA_PATH}/light_test_project DESTINATION data CONFIGURATIONS Release )
install( DIRECTORY ${DATA_PATH}/dynamic_concave_raycast DESTINATION data CONFIGURATIONS Release )
install( DIRECTORY ${DATA_PATH}/bike_project DESTINATION data CONFIGURATIONS Release )
install( FILES ${DATA_PATH}/env.env DESTINATION data )
install( FILES ${DATA_PATH}/multi_node.env DESTINATION data )
install( FILES ${DATA_PATH}/env_side_by_side.env DESTINATION data )
install( FILES ${DATA_PATH}/multi_window.env DESTINATION data )

# Copy start script
# Set variables for start scripts
set( PROGRAM_NAME hydra )
set( DEMO_DATA_PATH ../data )
set( PROJ_PATH_RELATIVE ogre_project/ogre.prj )

set( TEMP_DATA_DIR ${PROJECT_BINARY_DIR}/temp_data )

# Because CMake runs all commands here automatically when the file is included
# We need to create a temp dir for the config files.
# When make install is run the files are copied from temp_dir to install dir
if( WIN32 )
	configure_file( ${DATA_PATH}/shell_scripts/start.bat.in
		${TEMP_DATA_DIR}/start.bat )
else()
	configure_file( ${DATA_PATH}/shell_scripts/start.sh.in
		${TEMP_DATA_DIR}/start.sh )
endif()

# Install the configs from Temp dir
if( WIN32 )
	install( FILES ${TEMP_DATA_DIR}/start.bat DESTINATION bin CONFIGURATIONS Release )
else()
	install( FILES ${TEMP_DATA_DIR}/start.sh DESTINATION bin CONFIGURATIONS Release )
endif()

