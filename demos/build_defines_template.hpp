/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file build_defines_template.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_BULD_DEFINES_HPP
#define HYDRA_BULD_DEFINES_HPP

// Path additions, could also be just EXTRA_PATHS but this will make it easier
// to debug.
#define OGRE_BINARY_DIR ${OGRE_BINARY_DIR}
#define CEGUI_BINARY_DIR ${CEGUI_BINARY_DIR}
#define OIS_BINARY_DIR ${OIS_BINARY_DIR}
#define BOOST_BINARY_DIR ${BOOST_BINARY_DIR}
#define GENERAL_BINARY_DIR ${GENERAL_BINARY_DIR}
#define EXPAT_BINARY_DIR ${EXPAT_BINARY_DIR}
#define SKYX_BINARY_DIR ${SKYX_BINARY_DIR}
#define CAELUM_BINARY_DIR ${CAELUM_BINARY_DIR}

#define HYDRA_MAIN_BINARY_DIR ${HYDRA_MAIN_BINARY_DIR}
#define HYDRA_GL_BINARY_DIR ${HYDRA_GL_BINARY_DIR}

#endif	// HYDRA_BULD_DEFINES_HPP

