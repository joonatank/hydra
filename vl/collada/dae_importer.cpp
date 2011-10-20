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

Ogre::ColourValue convert_colour(COLLADAFW::Color const &col)
{ return Ogre::ColourValue(col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha()); }

/// Quick hack to handle colour texture types
/// We only support colours for now so this returns invalid for textutres.
Ogre::ColourValue convert_colour(COLLADAFW::ColorOrTexture const &col)
{
	if(col.isTexture())
	{
		return Ogre::ColourValue(-1, -1, -1, -1);
	}
	else if(col.isColor())
	{
		return convert_colour(col.getColor());
	}
	else
	{
		return Ogre::ColourValue(-1, -1, -1, -1);
	}
}

}

//--------------------------------------------------------------------
vl::dae::FileDeserializer::FileDeserializer(fs::path const &inputFile, 
	vl::SceneManagerPtr scene_manager, vl::MaterialManagerRefPtr material_man, bool flat_shading)
	: _input_file(inputFile)
	, _scene_manager(scene_manager)
	, _material_manager(material_man)
	, _flat_shading(flat_shading)
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
	// Create material instance map
	//std::map<COLLADAFW::MaterialId, std::string> instance_material_map;

	/// Attach entities
	/// @todo should rename Meshes also using instance names rather than the proto name
	for(std::vector<NodeEntityDefinition>::iterator iter =
		_node_entity_map.begin(); iter != _node_entity_map.end(); ++iter)
	{
		std::clog << "Mapping node " << iter->node->getName() << " to entity." << std::endl;
		std::map<COLLADAFW::UniqueId, vl::EntityPtr>::iterator l_iter = _entities.find(iter->geometry_id);
		if(l_iter == _entities.end())
		{
			std::clog << "Something really wrong! Couldn't find the entity." << std::endl;
		}
		else
		{
			std::clog << "Found the entity and attaching : " << l_iter->second->getName() 
				<< " to node " << iter->node->getName() << std::endl;
			iter->node->attachObject(l_iter->second);
		}
	}

	// Attach cameras
	for(std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> >::iterator iter =
		_node_camera_map.begin(); iter != _node_camera_map.end(); ++iter)
	{
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

	// Attach lights
	for(std::vector< std::pair<vl::SceneNodePtr, COLLADAFW::UniqueId> >::iterator iter =
		_node_light_map.begin(); iter != _node_light_map.end(); ++iter)
	{
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
	for(std::vector<MaterialMapEntry>::iterator iter =
		_materials.begin(); iter != _materials.end(); ++iter)
	{
		std::map<COLLADAFW::UniqueId, EffectMapEntry>::iterator l_iter = _effect_map.find(iter->effect_id);
		if(l_iter == _effect_map.end())
		{
			std::clog << "Something really wrong! Couldn't find the Effect." << std::endl;
		}
		else
		{
			vl::MaterialRefPtr material = l_iter->second.material;
			/// @todo needs to check the uniqueness of the name before resetting it
			/// collisions when using multiple import files
			/// This will of course be problematic for meshes... as they already said to use
			/// certain material. Ah well.
			material->setName(iter->name);
		}
	}

	// Remap submesh materials
	for(SubMeshMaterialIDMap::iterator iter = _submesh_material_map.begin();
		iter != _submesh_material_map.end(); ++iter)
	{
		std::map<COLLADAFW::MaterialId, COLLADAFW::UniqueId>::iterator mat_iter = _material_instance_map.find(iter->second);
		if(mat_iter != _material_instance_map.end())
		{
			bool found = false;
			for(size_t i = 0; i < _materials.size() && !found; ++i)
			{
				MaterialMapEntry const &material_node = _materials.at(i);
				if(material_node.material_id == mat_iter->second)
				{
					iter->first->setMaterial(material_node.name);
					found = true;
				}
			}

			if(!found)
			{
				std::clog << "Didn't find unique id in global materials map." << std::endl;
			}
		}
		else
		{
			std::clog << "Instance material id " << iter->second << " not found from material instance map." << std::endl;
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
	assert(parent);

	vl::SceneNodePtr node = parent->createChildSceneNode(nodeToWriter->getName());

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
		COLLADAFW::MaterialBindingArray const &materials = instance->getMaterialBindings();
		
		_node_entity_map.push_back(NodeEntityDefinition(parent, instance->getInstanciatedObjectId()));
		NodeEntityDefinition &def = _node_entity_map.back();
		for(size_t i = 0; i < materials.getCount(); ++i)
		{
			// Assuming that MaterialId is unique if not then we need to reverse the map (or use vector)
			_material_instance_map[materials[i].getMaterialId()] = materials[i].getReferencedMaterial();
		}
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
		MeshImporter meshImporter(_scene_manager->getMeshManager(), (COLLADAFW::Mesh*)geometry, _flat_shading);
		bool ret_val = meshImporter.write();
	
		// Here we could create the Entity but we don't have the node it should be mapped to
		// we could also use a map that we store all the meshes... or well we already have
		// one in MeshManager.

		// @todo
		// This has the problem that we are creating an instance here
		// though we should only create the mesh
		// then instance it when it's used
		_entities[geometry->getUniqueId()] = _scene_manager->createEntity(geometry->getName(), geometry->getName(), true);
		
		// Save the submeshes that need material remapping for later
		SubMeshMaterialIDMap const &map = meshImporter.getSubMeshMaterialMap();
		for(size_t i = 0; i < map.size(); ++i)
		{
			_submesh_material_map.push_back(map.at(i));
		}

		std::clog << "Loading mesh : " << geometry->getName() << " took " << t.elapsed() << std::endl;
		return ret_val;
	}
}

//--------------------------------------------------------------------
bool
vl::dae::FileDeserializer::writeMaterial( const COLLADAFW::Material* material )
{
	assert(_material_manager);

	_materials.push_back(MaterialMapEntry(material->getUniqueId(), material->getInstantiatedEffect(), material->getName() )); 

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
	if(col.isValid())
	{
		Ogre::ColourValue colour = convert_colour(col);
		std::clog << "Standard colour = " << colour << std::endl;
		mat->setDiffuse(colour);
	}

	// @todo add support for the real material information
	if(effect->getCommonEffects().getCount() > 1)
	{
		std::clog << "Single effect definition supported. Effect " << effect->getName() 
			<< " has multiple definitions. Taking the fist one." << std::endl;
	}

	if(effect->getCommonEffects().getCount() > 0)
	{
		std::clog << "Has a effects common." << std::endl;
		COLLADAFW::EffectCommon const *effect_common = effect->getCommonEffects()[0];

		COLLADAFW::EffectCommon::ShaderType const &shader = effect_common->getShaderType();
		std::string shader_name;
		switch(shader)
		{
			case COLLADAFW::EffectCommon::SHADER_UNKNOWN:
				break;
			case COLLADAFW::EffectCommon::SHADER_BLINN:
				shader_name = "Blinn";
				break;
			case COLLADAFW::EffectCommon::SHADER_CONSTANT:
				shader_name = "Constant";
				break;
			case COLLADAFW::EffectCommon::SHADER_PHONG:
				shader_name = "Phong";
				break;
			case COLLADAFW::EffectCommon::SHADER_LAMBERT:
				shader_name = "Lambert";
				break;
		}

		if(!shader_name.empty())
		{ std::clog << "Effect : " << effect->getName() << " has shader : " << shader_name << std::endl; }

		COLLADAFW::ColorOrTexture const &emission = effect_common->getEmission();
		COLLADAFW::ColorOrTexture const &ambient = effect_common->getAmbient();
		COLLADAFW::ColorOrTexture const &diffuse = effect_common->getDiffuse();
		COLLADAFW::ColorOrTexture const &specular = effect_common->getSpecular();
		COLLADAFW::FloatOrParam const &shininess = effect_common->getShininess();

		Ogre::ColourValue colour = convert_colour(emission);
		if(colour != Ogre::ColourValue(-1, -1, -1, -1))
		{
			mat->setEmissive(colour);
		}
		else
		{ std::clog << "Emission is invalid : probably a texture there which we don't yet support." << std::endl; }

		colour = convert_colour(ambient);
		if(colour != Ogre::ColourValue(-1, -1, -1, -1))
		{
			mat->setAmbient(colour);
		}
		else
		{ std::clog << "Ambient is invalid : probably a texture there which we don't yet support." << std::endl; }
		
		colour = convert_colour(diffuse);
		if(colour != Ogre::ColourValue(-1, -1, -1, -1))
		{
			mat->setDiffuse(colour);
		}
		else
		{ std::clog << "Diffuse is invalid : probably a texture there which we don't yet support." << std::endl; }

		colour = convert_colour(specular);
		if(colour != Ogre::ColourValue(-1, -1, -1, -1))
		{
			mat->setSpecular(colour);
		}
		else
		{ std::clog << "Diffuse is invalid : probably a texture there which we don't yet support." << std::endl; }

		if(shininess.getType() == COLLADAFW::FloatOrParam::FLOAT)
		{
			mat->setShininess(shininess.getFloatValue());
		}
		else
		{
			std::clog << "Shininess : PARAM type not supported." << std::endl;
		}

		/*	Not supported
		getReflective
		getReflectivity
		getOpacity
		getIndexOfRefraction
		getSamplerPointerArray
		*/
	}

	_effect_map[effect->getUniqueId()] = EffectMapEntry(effect->getUniqueId(), mat);

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
