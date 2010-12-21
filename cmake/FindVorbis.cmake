# Copyright (c) 2009, Whispersoft s.r.l.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
# * Neither the name of Whispersoft s.r.l. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Finds Vorbis library
#
#  Vorbis_INCLUDE_DIR 	- where to find vorbisenc.h, etc.
#  Vorbis_LIBRARY   	- The vorbis library. Both debug and optimized.
#  Vorbis_FILE_LIBRARY	- The vorbis file library. Both debug and optimized.
#  Vorbis_FOUND       	- True if Vorbis found.
#

if (Vorbis_INCLUDE_DIR)
	# Already in cache, be silent
	set(Vorbis_FIND_QUIETLY TRUE)
endif (Vorbis_INCLUDE_DIR)

find_path(Vorbis_INCLUDE_DIR vorbis/vorbisenc.h
	$ENV{VORBIS_HOME}/include
	/opt/local/include
	/usr/local/include
	/usr/include
	)

set(Vorbis_NAMES_RELEASE vorbis vorbis_static)
find_library(Vorbis_LIBRARY_RELEASE
	NAMES ${Vorbis_NAMES_RELEASE}
	PATHS /usr/lib /usr/local/lib /opt/local/lib $ENV{VORBIS_HOME}/lib
	)

set(Vorbis_NAMES_DEBUG vorbis_d vorbis_static_d )
find_library(Vorbis_LIBRARY_DEBUG
	NAMES ${Vorbis_NAMES_DEBUG}
	PATHS /usr/lib /usr/local/lib /opt/local/lib $ENV{VORBIS_HOME}/lib
	$ENV{VORBIS_HOME}
	)

if( Vorbis_LIBRARY_DEBUG AND NOT Vorbis_LIBRARY_RELEASE )
	set( Vorbis_LIBRARY_RELEASE ${Vorbis_LIBRARY_DEBUG} )
endif()

set(Vorbis_FILE_NAMES_RELEASE vorbisfile vorbisfile_static )
find_library(Vorbis_FILE_LIBRARY_RELEASE
	NAMES ${Vorbis_FILE_NAMES_RELEASE}
	PATHS /usr/lib /usr/local/lib /opt/local/lib $ENV{VORBIS_HOME}/lib
	$ENV{VORBIS_HOME}
	)

set(Vorbis_FILE_NAMES_DEBUG vorbisfile_d vorbisfile_static_d )
find_library(Vorbis_FILE_LIBRARY_DEBUG
	NAMES ${Vorbis_FILE_NAMES_DEBUG}
	PATHS /usr/lib /usr/local/lib /opt/local/lib $ENV{VORBIS_HOME}/lib
	$ENV{VORBIS_HOME}
	)

if( Vorbis_FILE_LIBRARY_DEBUG AND NOT Vorbis_FILE_LIBRARY_RELEASE )
	set( Vorbis_FILE_LIBRARY_RELEASE ${Vorbis_FILE_LIBRARY_DEBUG} )
endif()

set( Vorbis_LIBRARY debug ${Vorbis_LIBRARY_DEBUG}
		optimized ${Vorbis_LIBRARY_RELEASE} )

set( Vorbis_FILE_LIBRARY debug ${Vorbis_FILE_LIBRARY_DEBUG}
		optimized ${Vorbis_FILE_LIBRARY_RELEASE} )

if( Vorbis_INCLUDE_DIR AND Vorbis_LIBRARY )
	set( Vorbis_FOUND TRUE )
	set( Vorbis_LIBRARIES ${Vorbis_LIBRARY} )
else()
	set( Vorbis_FOUND FALSE )
	set( Vorbis_LIBRARIES )
endif()

if( Vorbis_FOUND )
	if( NOT Vorbis_FIND_QUIETLY )
		message( STATUS "Found Vorbis: ${Vorbis_LIBRARY}" )
		if( Vorbis_FILE_LIBRARY )
			message( STATUS "Found Vorbisfile: ${Vorbis_FILE_LIBRARY}" )
		endif()
	endif( NOT Vorbis_FIND_QUIETLY )
else( Vorbis_FOUND )
	if( Vorbis_FIND_REQUIRED )
		message( STATUS "Looked for Vorbis libraries named ${Vorbis_NAMES}." )
		message(STATUS "Include file detected: [${Vorbis_INCLUDE_DIR}].")
		message(STATUS "Lib file detected: [${Vorbis_LIBRARY}].")
		message(FATAL_ERROR "=========> Could NOT find Vorbis library")
	endif (Vorbis_FIND_REQUIRED)
endif( Vorbis_FOUND )

mark_as_advanced(
	Vorbis_LIBRARY
	Vorbis_LIBRARY_RELEASE
	Vorbis_LIBRARY_DEBUG
	Vorbis_FILE_LIBRARY
	Vorbis_FILE_LIBRARY_RELEASE
	Vorbis_FILE_LIBRARY_DEBUG
	Vorbis_INCLUDE_DIR
	)

