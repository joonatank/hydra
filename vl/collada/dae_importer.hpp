/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_importer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_COLLADA_DAE_IMPORTER_HPP
#define HYDRA_COLLADA_DAE_IMPORTER_HPP

#include <COLLADAFramework/COLLADAFWIWriter.h>
#include <COLLADAFramework/COLLADAFWUniqueId.h>
#include <COLLADAFramework/COLLADAFWVisualScene.h>
#include <COLLADAFramework/COLLADAFWLibraryNodes.h>
#include <COLLADAFramework/COLLADAFWMaterial.h>
#include <COLLADAFramework/COLLADAFWEffect.h>
#include <COLLADAFramework/COLLADAFWImage.h>
#include <COLLADAFramework/COLLADAFWMaterialBinding.h>

#include "COLLADABUURI.h"
#include "Math/COLLADABUMathMatrix4.h"

#include <stack>
#include <list>
#include <map>

/// Necessary because we pass paths to here
#include "base/filesystem.hpp"
/// Necessary because the scene graph is straightly read here
#include "scene_manager.hpp"

/// Necessary for importing a mesh
#include "dae_mesh_importer.hpp"

#include "dae_common.hpp"

namespace vl
{

namespace dae
{

struct ImporterSettings
{
	// How to calculate normals for shading
	enum SHADING
	{
		SMOOTH,		// calculate smooth normals everywhere
		FLAT,		// every edge is sharp
		COMBINED,	// use cut angle to smooth normals
	};

	SHADING shading;

	Ogre::Radian normals_cut_angle;

	// where to attach all scene nodes (root)
	vl::SceneNodePtr attach_node;

	// what material to use as a base for all materials
	std::string base_material;

	/// @brief checks all materials for exact duplicates and removes them
	/// exact duplicate here is all parameters except name
	bool remove_duplicate_materials;

	// traverses all the meshes removing exact duplicates
	// @todo not implemented
	bool remove_duplicate_meshes;
};

/// @class DaeReader
/// @brief class used read dae files to the Hydra runtime
/// Remapping the name of writer used by OpenCollada to Reader (or deserializer) as used in Hydra.
class Importer : public COLLADAFW::IWriter
{
public:
	/// Constructor
	Importer(ImporterSettings const &settings, Managers const &managers);

	/// Destructor
	virtual ~Importer();

	bool read(fs::path const &input_file);


	/// Virtual overrides
public:

	/** Deletes the entire scene.
	@param errorMessage A message containing informations about the error that occurred.
	*/
	void cancel(std::string const &errorMessage);;

	/** Prepare to receive data.*/
	void start();;

	/** Remove all objects that don't have an object. Deletes unused visual scenes.*/
	void finish();;

	/** When this method is called, the writer must write the global document asset.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeGlobalAsset ( const COLLADAFW::FileInfo* asset );

	/** Writes the entire visual scene.
	@return True on succeeded, false otherwise.*/
	virtual bool writeVisualScene ( const COLLADAFW::VisualScene* visualScene );

	/** Writes the scene.
	@return True on succeeded, false otherwise.*/
	virtual bool writeScene ( const COLLADAFW::Scene* scene );

	/** Handles all nodes in the library nodes.
	@return True on succeeded, false otherwise.*/
	virtual bool writeLibraryNodes( const COLLADAFW::LibraryNodes* libraryNodes );

	/** Writes the geometry.
	@return True on succeeded, false otherwise.*/
	virtual bool writeGeometry ( const COLLADAFW::Geometry* geometry );

	/** Writes the material.
	@return True on succeeded, false otherwise.*/
	virtual bool writeMaterial( const COLLADAFW::Material* material );

	/** Writes the effect.
	@return True on succeeded, false otherwise.*/
	virtual bool writeEffect( const COLLADAFW::Effect* effect );

	/** Writes the camera.
	@return True on succeeded, false otherwise.*/
	virtual bool writeCamera( const COLLADAFW::Camera* camera );

	/** Writes the image.
	@return True on succeeded, false otherwise.*/
	virtual bool writeImage( const COLLADAFW::Image* image );

	/** Writes the light.
	@return True on succeeded, false otherwise.*/
	virtual bool writeLight( const COLLADAFW::Light* light );

	/** Writes the animation.
	@return True on succeeded, false otherwise.*/
	virtual bool writeAnimation( const COLLADAFW::Animation* animation );

	/** Writes the animation.
	@return True on succeeded, false otherwise.*/
	virtual bool writeAnimationList( const COLLADAFW::AnimationList* animationList );

	/** Writes the skin controller data.
	@return True on succeeded, false otherwise.*/
	virtual bool writeSkinControllerData( const COLLADAFW::SkinControllerData* skinControllerData );

	/** Writes the controller.
	@return True on succeeded, false otherwise.*/
	virtual bool writeController( const COLLADAFW::Controller* Controller );

	/** When this method is called, the writer must write the formulas. All the formulas of the entire
	COLLADA file are contained in @a formulas.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeFormulas( const COLLADAFW::Formulas* formulas ){return true;}

	/** When this method is called, the writer must write the kinematics scene. 
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeKinematicsScene( const COLLADAFW::KinematicsScene* kinematicsScene ){return true;};


private :
	/// SceneGraph
	bool writeNodes(COLLADAFW::NodePointerArray const &nodesToWriter, vl::SceneNodePtr parent);

	bool writeNode(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent);

	/// Individual handlers
	void handleInstanceGeometries(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent);

	void handleInstanceCameras(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent);

	void handleInstanceLights(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent);
	
	void handleInstanceNodes(COLLADAFW::InstanceNodePointerArray const &instanceNodes);

	/** Disable default copy ctor. */
	Importer(Importer const &);
    /** Disable default assignment operator. */
	Importer &operator= (Importer const &);

	/// Data
private:
	vl::SceneManagerPtr _scene_manager;
	vl::MaterialManagerRefPtr _material_manager;

	struct NodeEntityDefinition
	{
		NodeEntityDefinition(void)
			: node(0)
			, geometry_id()
		{}

		NodeEntityDefinition(vl::SceneNodePtr n, COLLADAFW::UniqueId g_id)
			: node(n), geometry_id(g_id)
		{}

		vl::SceneNodePtr node;
		COLLADAFW::UniqueId geometry_id;
	};

	struct EffectMapEntry
	{
		EffectMapEntry(void)
		{}

		EffectMapEntry(COLLADAFW::UniqueId const &id, MaterialRefPtr mat)
			: effect_id(id), material(mat)
		{}

		COLLADAFW::UniqueId effect_id;
		MaterialRefPtr material;
	};

	struct MaterialMapEntry
	{
		MaterialMapEntry(void)
		{}

		MaterialMapEntry(COLLADAFW::UniqueId const &mat_id, COLLADAFW::UniqueId const &eff_id, std::string const &n)
			: material_id(mat_id), effect_id(eff_id), name(n)
		{}

		COLLADAFW::UniqueId material_id;
		COLLADAFW::UniqueId effect_id;
		std::string name;
	};


	// Maps for instances, these are handled at the end of file reading
	// and used to map the concrete objects (_cameras, _entities, _lights) to
	// their parent nodes.
	std::vector<NodeEntityDefinition> _node_entity_map;
	std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> > _node_camera_map;
	std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> > _node_light_map;
	std::map<COLLADAFW::UniqueId, vl::CameraPtr> _cameras;
	std::map<COLLADAFW::UniqueId, vl::EntityPtr> _entities;
	std::map<COLLADAFW::UniqueId, vl::LightPtr> _lights;
	SubMeshMaterialIDMap _submesh_material_map;

	/// Map which maps effect id to an information node
	std::map<COLLADAFW::UniqueId, EffectMapEntry> _effect_map;
	std::vector<MaterialMapEntry> _materials;

	std::map<COLLADAFW::MaterialId, COLLADAFW::UniqueId> _material_instance_map;

	bool _flat_shading;
};

}	// namespace dae

}	// namespace vl


#endif	// HYDRA_COLLADA_DAE_IMPORTER_HPP
