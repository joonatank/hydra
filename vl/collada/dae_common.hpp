/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_common.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_COLLADA_DAE_COMMON_HPP
#define HYDRA_COLLADA_DAE_COMMON_HPP

#include "typedefs.hpp"

namespace vl
{

namespace dae
{

struct Managers
{
	vl::SceneManagerPtr scene;
	vl::MeshManagerRefPtr mesh;
	vl::MaterialManagerRefPtr material;

	// both kinematic and physics so that the exporter/importer can choose
	vl::KinematicWorldRefPtr kinematic;
	vl::physics::WorldRefPtr physics;
};

}	// namespace dae

}	// namespace vl

#endif	// HYDRA_COLLADA_DAE_COMMON_HPP
