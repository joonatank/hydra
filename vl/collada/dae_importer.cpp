/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-11
 *	@file collada/dae_importer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *
 
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "dae_importer.hpp"

#include <COLLADASaxFrameworkLoader/COLLADASaxFWLLoader.h>

#include <COLLADAFramework/COLLADAFWRoot.h>
#include <COLLADAFramework/COLLADAFWGeometry.h>
#include <COLLADAFramework/COLLADAFWNode.h>
#include <COLLADAFramework/COLLADAFWVisualScene.h>
#include <COLLADAFramework/COLLADAFWInstanceGeometry.h>
#include <COLLADAFramework/COLLADAFWLight.h>
#include <COLLADAFramework/COLLADAFWMaterial.h>
#include <COLLADAFramework/COLLADAFWCamera.h>
#include <COLLADAFramework/COLLADAFWGeometry.h>

#include <iostream>

/// Necessary for importing a mesh
#include "dae_mesh_importer.hpp"

/// Necessary for creating SceneNodes
#include "scene_node.hpp"
#include "scene_manager.hpp"
/// Necessary for creating various SceneObjects
#include "light.hpp"
#include "camera.hpp"
#include "entity.hpp"

/// Necessary for creating materials
#include "material_manager.hpp"
#include "material.hpp"

namespace {

Ogre::Vector3 convert_vec(COLLADABU::Math::Vector3 const &v)
{ return Ogre::Vector3(v.x, v.y, v.z); }
Ogre::Quaternion convert_quat(COLLADABU::Math::Quaternion const &q)
{ return Ogre::Quaternion(q.w, q.x, q.y, q.z); }

}

//--------------------------------------------------------------------
vl::dae::FileDeserializer::FileDeserializer(fs::path const &inputFile, 
	vl::SceneManagerPtr scene_manager, vl::MaterialManagerRefPtr material_man)
	: _input_file(inputFile)
	, _scene_manager(scene_manager)
	, _material_manager(material_man)
{
	assert(_scene_manager);
	assert(_material_manager);
}

//--------------------------------------------------------------------
vl::dae::FileDeserializer::~FileDeserializer()
{}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::write()
{
	COLLADASaxFWL::Loader loader;
	COLLADAFW::Root root(&loader, this);

	// Load scene graph 
	if ( !root.loadDocument(_input_file.string()) )
		return false;

	return true;
}

//--------------------------------------------------------------------
void
vl::dae::FileDeserializer::cancel(std::string const &errorMessage )
{
}

//--------------------------------------------------------------------
void
vl::dae::FileDeserializer::start()
{
}

//--------------------------------------------------------------------
void
vl::dae::FileDeserializer::finish()
{
	/// Do the mapping
	for(std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> >::iterator iter =
		_node_entity_map.begin(); iter != _node_entity_map.end(); ++iter)
	{
		std::clog << "Mapping node " << iter->first->getName() << " to entity." << std::endl;
		std::map<COLLADAFW::UniqueId, vl::EntityPtr>::iterator l_iter = _entities.find(iter->second);
		if(l_iter == _entities.end())
		{
			std::clog << "Something really wrong! Couldn't find the entity." << std::endl;
		}
		else
		{
			std::clog << "Found the entity and attaching : " << l_iter->second->getName() 
				<< " to node " << iter->first->getName() << std::endl;
			iter->first->attachObject(l_iter->second);
		}
	}

	for(std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> >::iterator iter =
		_node_camera_map.begin(); iter != _node_camera_map.end(); ++iter)
	{
		std::clog << "Mapping node " << iter->first->getName() << " to camera." << std::endl;
		std::map<COLLADAFW::UniqueId, vl::CameraPtr>::iterator l_iter = _cameras.find(iter->second);
		if(l_iter == _cameras.end())
		{
			std::clog << "Something really wrong! Couldn't find the camera." << std::endl;
		}
		else
		{
			std::clog << "Found the camera and attaching : " << l_iter->second->getName() 
				<< " to node " << iter->first->getName() << std::endl;
			iter->first->attachObject(l_iter->second);
		}
	}

	for(std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> >::iterator iter =
		_node_light_map.begin(); iter != _node_light_map.end(); ++iter)
	{
		std::clog << "Mapping node " << iter->first->getName() << " to light." << std::endl;
		std::map<COLLADAFW::UniqueId, vl::LightPtr>::iterator l_iter = _lights.find(iter->second);
		if(l_iter == _lights.end())
		{
			std::clog << "Something really wrong! Couldn't find the light." << std::endl;
		}
		else
		{
			std::clog << "Found the light and attaching : " << l_iter->second->getName() 
				<< " to node " << iter->first->getName() << std::endl;
			iter->first->attachObject(l_iter->second);
		}
	}

	// Rename materials
	for(std::vector< std::pair<COLLADAFW::UniqueId, std::string> >::iterator iter =
		_materials.begin(); iter != _materials.end(); ++iter)
	{
		std::clog << "Renaming material " << iter->second << std::endl;
		std::map<COLLADAFW::UniqueId, vl::MaterialRefPtr>::iterator l_iter = _effect_map.find(iter->first);
		if(l_iter == _effect_map.end())
		{
			std::clog << "Something really wrong! Couldn't find the Effect." << std::endl;
		}
		else
		{
			/// @todo needs to check the uniqueness of the name before resetting it
			/// collisions when using multiple import files
			/// This will of course be problematic for meshes... as they already said to use
			/// certain material. Ah well.
			std::clog << "Found the effect and renamed : " << l_iter->second->getName() 
				<< " to " << iter->second << std::endl;
			l_iter->second->setName(iter->second);
		}
	}
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeGlobalAsset( const COLLADAFW::FileInfo* asset )
{
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeVisualScene(COLLADAFW::VisualScene const *visualScene)
{
	assert(visualScene);

	std::clog << "vl::dae::FileDeserializer::write : importing visual scene" << std::endl;

	writeNodes(visualScene->getRootNodes(), _scene_manager->getRootSceneNode());

	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeScene( const COLLADAFW::Scene* scene )
{
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeLibraryNodes( const COLLADAFW::LibraryNodes* libraryNodes )
{
	std::clog << "vl::dae::FileDeserializer::writeLibraryNodes" << std::endl;
//	mLibraryNodesList.push_back(*libraryNodes);
	return true;
}

//------------------------------
bool
vl::dae::FileDeserializer::writeNodes(COLLADAFW::NodePointerArray const &nodesToWriter, vl::SceneNodePtr parent)
{
	std::clog << "vl::dae::FileDeserializer::writeNodes" << std::endl;
	for ( size_t i = 0, count = nodesToWriter.getCount(); i < count; ++i)
	{
		writeNode(nodesToWriter[i], parent);
	}
	return true;
}

//------------------------------
bool
vl::dae::FileDeserializer::writeNode(const COLLADAFW::Node* nodeToWriter, vl::SceneNodePtr parent)
{
	std::clog << "vl::dae::FileDeserializer::writeNode" << std::endl;

	assert(parent);

	vl::SceneNodePtr node = parent->createChildSceneNode(nodeToWriter->getName());
	std::clog << "Created node " << node->getName() << std::endl;
	// copy transformation matrix
	COLLADABU::Math::Matrix4 mat = nodeToWriter->getTransformationMatrix();
	node->setOrientation(convert_quat(mat.extractQuaternion()));
	node->setPosition(convert_vec(mat.getTrans()));
	node->setScale(convert_vec(mat.getScale()));

	// copy visibility

	// store the id to id -> SceneNode map so that Camera and Entity processors can use them

	/// store the ids so we can do a mapping phase
	if(nodeToWriter->getInstanceGeometries().getCount() != 0)
	{
		handleInstanceGeometries(nodeToWriter, node);
	}

	if(nodeToWriter->getInstanceCameras().getCount() != 0)
	{
		handleInstanceCameras(nodeToWriter, node);
	}

	if(nodeToWriter->getInstanceLights().getCount() != 0)
	{
		handleInstanceLights(nodeToWriter, node);
	}
	
	if(nodeToWriter->getInstanceControllers().getCount() != 0)
	{
		std::clog << "Node " << node->getName() << " has instance controllers which are not supported!" << std::endl;
	}
	
	writeNodes(nodeToWriter->getChildNodes(), node);

	if(nodeToWriter->getInstanceNodes().getCount() != 0)
	{
		handleInstanceNodes(nodeToWriter->getInstanceNodes());
	}

	return true;
}

void
vl::dae::FileDeserializer::handleInstanceGeometries(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent)
{
	for(size_t i = 0; i < nodeToWriter->getInstanceGeometries().getCount(); ++i)
	{
		COLLADAFW::InstanceGeometry *instance = nodeToWriter->getInstanceGeometries()[i];
		_node_entity_map.push_back(std::make_pair(parent, instance->getInstanciatedObjectId()));
	}
}

void
vl::dae::FileDeserializer::handleInstanceCameras(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent)
{
	for(size_t i = 0; i < nodeToWriter->getInstanceCameras().getCount(); ++i)
	{
		COLLADAFW::InstanceCamera *instance = nodeToWriter->getInstanceCameras()[i];
		_node_camera_map.push_back(std::make_pair(parent, instance->getInstanciatedObjectId()));
	}
}

void
vl::dae::FileDeserializer::handleInstanceLights(COLLADAFW::Node const *nodeToWriter, vl::SceneNodePtr parent)
{
	for(size_t i = 0; i < nodeToWriter->getInstanceLights().getCount(); ++i)
	{
		COLLADAFW::InstanceLight *instance = nodeToWriter->getInstanceLights()[i];
		_node_light_map.push_back(std::make_pair(parent, instance->getInstanciatedObjectId()));
	}
}

//------------------------------
void
vl::dae::FileDeserializer::handleInstanceNodes(COLLADAFW::InstanceNodePointerArray const & instanceNodes)
{
	std::clog << "vl::dae::FileDeserializer::handleInstanceNodes : NOT SUPPORTED!" << std::endl;
	/*
	for ( size_t i = 0, count = instanceNodes.getCount(); i < count; ++i)
	{
		const COLLADAFW::InstanceNode* instanceNode = instanceNodes[i];
		const COLLADAFW::UniqueId& referencedNodeUniqueId = instanceNode->getInstanciatedObjectId();
		UniqueIdNodeMap::const_iterator it = mUniqueIdNodeMap.find( referencedNodeUniqueId );
		if ( it != mUniqueIdNodeMap.end() )
		{
			const COLLADAFW::Node* referencedNode = it->second;
			writeNode(referencedNode);
		}
	}
	*/
}


//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeGeometry(COLLADAFW::Geometry const *geometry)
{
	std::clog << "vl::dae::FileDeserializer::writeGeometry" << std::endl;
	if ( geometry->getType() != COLLADAFW::Geometry::GEO_TYPE_MESH )
	{
		std::clog << "vl::dae::FileDeserializer::writeGeometry " 
			<< ": Unhandled Geometry type " << geometry->getType() << std::endl;
		return true;
	}
	else
	{
		vl::timer t;
		// @todo mesh writer should have the name we store the mesh
		// or we could pass it from here
		// this because Collada does not guerantie that every object has unique
		// names and they can even be unamed
		// in which case we need to use the unique id for the name.
		MeshImporter meshImporter(_scene_manager->getMeshManager(), (COLLADAFW::Mesh*)geometry );
		bool ret_val = meshImporter.write();
	
		// Here we could create the Entity but we don't have the node it should be mapped to
		// we could also use a map that we store all the meshes... or well we already have
		// one in MeshManager.

		// @todo
		// This has the problem that we are creating an instance here
		// though we should only create the mesh
		// then instance it when it's used
		_entities[geometry->getUniqueId()] = _scene_manager->createEntity(geometry->getName(), geometry->getName(), true);
		
		std::clog << "Loading mesh : " << geometry->getName() << " took " << t.elapsed() << std::endl;
		return ret_val;
	}
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeMaterial( const COLLADAFW::Material* material )
{
	std::clog << "vl::dae::FileDeserializer::writeMaterial" << std::endl;
	assert(_material_manager);

	_materials.push_back(std::make_pair(material->getInstantiatedEffect(), material->getName())); 

	//	mUniqueIdFWMaterialMap.insert(std::make_pair(material->getUniqueId(), *material ));
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeEffect( const COLLADAFW::Effect* effect )
{
	std::clog << "vl::dae::FileDeserializer::writeEffect" << std::endl;

	MaterialRefPtr mat = _material_manager->createMaterial(effect->getName());

	COLLADAFW::Color const &col = effect->getStandardColor();
	mat->setDiffuse(Ogre::ColourValue(col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha()));

	// @todo add support for the real material information

	_effect_map[effect->getUniqueId()] = mat;

	// @todo add the material to stack? do we need this?
	// we will anyway use material names not pointers to them.
	// Ogre takes care of that.

//	mUniqueIdFWEffectMap.insert(std::make_pair(effect->getUniqueId(), *effect ));
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeCamera(COLLADAFW::Camera const *camera)
{
	std::clog << "vl::dae::FileDeserializer::writeCamera" << std::endl;

	// @todo
	// This has the problem that we are creating an instance here
	// though we should only create a template
	// probably can't be avoided with our current architecture so
	// we need to use clone when creating more than one instance
	vl::CameraPtr cam = _scene_manager->createCamera(camera->getName());

	// Copy clipping

	// Copy other attributes?

	_cameras[camera->getUniqueId()] = cam;

	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeImage( const COLLADAFW::Image* image )
{
	std::clog << "vl::dae::FileDeserializer::writeImage : NOT IMPLEMENTED" << std::endl;
//	mUniqueIdFWImageMap.insert(std::make_pair(image->getUniqueId(),*image ));
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeLight(COLLADAFW::Light const *light)
{
	std::clog << "vl::dae::FileDeserializer::writeLight" << std::endl;

	// @todo
	// This has the problem that we are creating an instance here
	// though we should only create a template
	// probably can't be avoided with our current architecture so
	// we need to use clone when creating more than one instance
	vl::LightPtr lig = _scene_manager->createLight(light->getName());

	// Copy type

	// Copy colour (diffuse and specular)

	// Copy cast shadows (if there is such)

	// Copy visible

	// Copy spotlight params
	
	// Copy attenuation

	_lights[light->getUniqueId()] = lig;

	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeAnimation( const COLLADAFW::Animation* animation )
{
	std::clog << "vl::dae::FileDeserializer::writeAnimation : NOT IMPLEMENTED" << std::endl;
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeAnimationList( const COLLADAFW::AnimationList* animationList )
{
	std::clog << "vl::dae::FileDeserializer::writeAnimationList : NOT IMPLEMENTED" << std::endl;
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeSkinControllerData( const COLLADAFW::SkinControllerData* skinControllerData )
{
	std::clog << "vl::dae::FileDeserializer::writeSkinControllerData : NOT IMPLEMENTED" << std::endl;
	return true;
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeController( const COLLADAFW::Controller* Controller )
{
	std::clog << "vl::dae::FileDeserializer::writeController : NOT IMPLEMENTED" << std::endl;
	return true;
}
