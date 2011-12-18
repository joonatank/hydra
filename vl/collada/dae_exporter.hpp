/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_exporter.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_COLLADA_DAE_EXPORTER_HPP
#define HYDRA_COLLADA_DAE_EXPORTER_HPP

/**	Stub file
 *	Descripes the architecture and tools necessary for implementation 
 *	but for now does not implement the Exporter.
 *
 *	Using COLLADASW::StreamWriter
 *	Copy the implementation architecture from Blender/collada/DocumentExporter
 */

#include "dae_common.hpp"

#include "base/filesystem.hpp"

namespace vl
{

namespace dae
{

struct ExporterSettings
{

};

class Exporter
{
public :
	Exporter(ExporterSettings const &settings, Managers const &managers);

	~Exporter(void);

	void write(fs::path const &file_name);

private :
	/** Disable default copy ctor. */
	Exporter(Exporter const &);
    /** Disable default assignment operator. */
	Exporter &operator= (Exporter const &);

private :
	vl::SceneManagerPtr _scene_manager;
	vl::MeshManagerRefPtr _mesh_manager;
	vl::MaterialManagerRefPtr _material_manager;

};	// class Exporter


}	// namespace dae

}	// namespace vl

#endif	// HYDRA_COLLADA_DAE_EXPORTER_HPP