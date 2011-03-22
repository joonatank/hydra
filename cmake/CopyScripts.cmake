
## Copy a single script ##
# script path
# script name
# output name
MACRO( copy_shell_script script_path script_name output_name )

if( WIN32 )
	set( extension bat )
else()
	set( extension sh )
endif()
set( input_script ${script_path}/${script_name}.${extension}.in )
set( script_file ${output_name}.${extension} )

if( CMAKE_CONFIGURATION_TYPES )
	foreach( e ${CMAKE_CONFIGURATION_TYPES} )
		set( output ${PROJECT_BINARY_DIR}/${e}/${script_file} )
		set( BUILD_TYPE ${e} )
		configure_file( ${input_script} ${output} )
	endforeach()
else()
	set( output ${PROJECT_BINARY_DIR}/${script_file} )
	configure_file( ${input_script} ${output} )
endif()


ENDMACRO()

## Copy all start scripts macro ##
MACRO( copy_start_scripts program_name project_path start_suffix )

message( STATUS "Copying start scripts for CMake project = ${PROJECT_NAME}"
	" : exe name = ${program_name} : project file ${project_path}" )

# Copy the single Node start script
set( PROGRAM_NAME ${program_name} )
set( DEMO_DATA_PATH ${vl_SOURCE_DIR}/data )
set( PROJ_PATH_RELATIVE ${project_path} )
# TODO the LIB_DIR should be user configurable
# TODO we need to add Debug and Release support
if( WIN32 )
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set( LIB_DIR "d:\\libs" )
	else()
		set( LIB_DIR "d:\\libs32" )
	endif()
endif()

# TODO should also copy the eqc file so that the hostname and port can be set
# correctly there also.

set( script_path ${DEMO_DATA_PATH}/shell_scripts )
copy_shell_script( ${script_path} start start_${start_suffix} )
copy_shell_script( ${script_path} start_multi_window start_multi_window_${start_suffix} )

# Copy the multi node listening start script
set( SERVER_HOSTNAME localhost )
set( SERVER_PORT 4699 )

set( script_path ${DEMO_DATA_PATH}/shell_scripts )
set( script_name start_multi_node )
set( output_name start_multi_${start_suffix} )
copy_shell_script( ${script_path} start_multi_node start_multi_node_${start_suffix} )

ENDMACRO()

