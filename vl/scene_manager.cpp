/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file scene_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "scene_manager.hpp"

#include "scene_node.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "movable_text.hpp"
#include "ray_object.hpp"

/// Necessary for better shadow camera
#include <OGRE/OgreShadowCameraSetupLiSPSM.h>
#include <OGRE/OgreShadowCameraSetupPlaneOptimal.h>
#include <OGRE/OgreShadowCameraSetupPSSM.h>

// Necessary for Rendering system harware capabilities
#include <OGRE/OgreRoot.h>

#include "logger.hpp"

// Necessary for Different sky plugins
#include "sky_skyx.hpp"
#include "sky_caelum.hpp"

#include "math/math.hpp"

const char *vl::EDITOR_CAMERA = "editor/perspective";

Ogre::FogMode
getOgreFogMode(vl::FogMode m)
{
	switch(m)
	{
	case vl::FOG_NONE:
		return Ogre::FOG_NONE;
	case vl::FOG_LINEAR:
		return Ogre::FOG_LINEAR;
	case vl::FOG_EXP:
		return Ogre::FOG_EXP;
	case vl::FOG_EXP2:
		return Ogre::FOG_EXP2;
	default:
		return Ogre::FOG_NONE;
	}
}


/// ------------------------------ ShadowInfo --------------------------------
vl::ShadowInfo::ShadowInfo(std::string const &cam)
	: _camera(cam)
	, _enabled(false)
	, _texture_size(1024)
	, _max_distance(250)
	, _caster_material("ShadowCaster")
	, _shelf_shadow(true)
	, _dir_light_extrusion_distance(100)
	, _dirty(true)
{
}

void
vl::ShadowInfo::setEnabled(bool enabled)
{
	if(enabled != _enabled)
	{
		_enabled = enabled;
		_setDirty();
	}
}

void 
vl::ShadowInfo::setCamera(std::string const &str)
{
	std::string name(str);
	vl::to_lower(name);
	if(name != _camera)
	{
		_setDirty();
		_camera = name;
	}
}

void
vl::ShadowInfo::setTextureSize(int const size)
{
	int final_size = size;
	// default to 128 texture
	if(size < 128)
	{
		final_size = 128;
		std::clog << "vl::ShadowInfo::setTextureSize : " << size 
			<< " is too small. Trying to fix the problem by using "
			<< final_size << " instead." << std::endl;
	}
	// square textures thank you very much
	else if(!is_power_of_two(size))
	{
		final_size = next_power_of_two(size);
		std::clog << "vl::ShadowInfo::setTextureSize : " << size 
			<< " is not power of two. Trying to fix the problem by using "
			<< final_size << " instead." << std::endl;
	}

	if(_texture_size != final_size)
	{
		_texture_size = final_size;
		_setDirty();
	}
}

void
vl::ShadowInfo::setMaxDistance(vl::scalar const dist)
{
	if(_max_distance != dist)
	{
		_max_distance = dist;
		_setDirty();
	}

}

void
vl::ShadowInfo::setShadowCasterMaterial(std::string const &material_name)
{
	if(material_name != _caster_material)
	{
		_caster_material = material_name;
		_setDirty();
	}
}

void
vl::ShadowInfo::setShelfShadowEnabled(bool enable)
{
	if(enable != _shelf_shadow)
	{
		_shelf_shadow = enable;
		_setDirty();
	}
}

void
vl::ShadowInfo::setDirLightExtrusionDistance(vl::scalar const dist)
{
	if(dist != _dir_light_extrusion_distance)
	{
		_dir_light_extrusion_distance = dist;
		_setDirty();
	}
}

/// ------------------------------ SkyInfo -----------------------------------
vl::SkyInfo::SkyInfo(std::string const &preset_name)
	: _preset(preset_name)
	, _dirty(true)
{
	vl::to_lower(_preset);
}

void
vl::SkyInfo::setPreset(std::string const &preset_name)
{
	if(preset_name != _preset)
	{
		_preset = preset_name;
		vl::to_lower(_preset);
		_setDirty();
	}
}


/// ------------------------------ SceneManager ------------------------------
/// Public
/// Master constructor
vl::SceneManager::SceneManager(vl::Session *session, vl::MeshManagerRefPtr mesh_man)
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _session(session)
	, _mesh_manager(mesh_man)
	, _ogre_sm(0)
	, _sky_sim(0)
{
	std::cout << vl::TRACE << "vl::SceneManager::SceneManager" << std::endl;

	_session->registerObject( this, OBJ_SCENE_MANAGER);
	_root = createFreeSceneNode("Root");

	SceneNodePtr camera = _root->createChildSceneNode(EDITOR_CAMERA);
	CameraPtr cam = createCamera(EDITOR_CAMERA);
	camera->attachObject(cam);
	// @todo the position and orientation should be such that it can see the whole scene
	// this needs bounding box calculations which are not available in the master copy
	camera->setPosition(Ogre::Vector3(0, 3, 15));
	// @todo we need some nice way of moving the camera
	// we could use the state system, when the state is EDITOR the camera movements are
	// applied to editor camera otherwise they are passed to the user processing
}

/// Renderer constructor
vl::SceneManager::SceneManager(vl::Session *session, uint64_t id, Ogre::SceneManager *native, vl::MeshManagerRefPtr mesh_man)
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _session(session)
	, _mesh_manager(mesh_man)
	, _ogre_sm(native)
	, _sky_sim(0)
{
	std::cout << vl::TRACE << "vl::SceneManager::SceneManager" << std::endl;

	assert(_session);
	assert(id != vl::ID_UNDEFINED );
	assert(_ogre_sm);

	_session->registerObject( this, OBJ_SCENE_MANAGER, id );

	// SkyX::AtmosphereManager::Options(9.77501f, 10.2963f, 0.01f, 0.0022f, 0.000675f, 30, Ogre::Vector3(0.57f, 0.52f, 0.44f), -0.991f, 3, 4);
	/// Add skyX presets hard coded for now
	_sky_presets["sunset"] = vl::SkySettings(Ogre::Vector3(8.85f, 7.5f, 20.5f),  -0.08f, 0, false, true, 300, false, Ogre::Vector3::NEGATIVE_UNIT_X, Ogre::ColourValue(0.63f,0.63f,0.7f), Ogre::ColourValue(0.35, 0.2, 0.92, 0.1), Ogre::ColourValue(0.4, 0.7, 0, 0), Ogre::Vector2(0.8,1));
	_sky_presets["clear"] = vl::SkySettings(Ogre::Vector3(17.16f, 7.5f, 20.5f), 0, 0, false, false);
	_sky_presets["thunderstorm"] = vl::SkySettings(Ogre::Vector3(12.23, 7.5f, 20.5f),  0, 0, false, true, 300, false, Ogre::Vector3::UNIT_Z, Ogre::ColourValue(0.63f,0.63f,0.7f), Ogre::ColourValue(0.25, 0.4, 0.5, 0.1), Ogre::ColourValue(0.45, 0.3, 0.6, 0.1), Ogre::Vector2(1,1));
	_sky_presets["desert"] = vl::SkySettings(Ogre::Vector3(7.59f, 7.5f, 20.5f), 0, -0.8f, true, false);
	_sky_presets["night"] = vl::SkySettings(Ogre::Vector3(21.5f, 7.5, 20.5), 0.03, -0.25, true, false);
}

vl::SceneManager::~SceneManager( void )
{
	// @todo this is rather complex compared to what it needs to be
	// we never need to send messages or gather ids here because it only matters
	// on Master and if the master scene manager is destroyed all the
	// slaves need to destroy their scene managers also.
	// More simpler version that does not handle distribution of SceneNodes
	// This would allow us to destroy the SceneManager distribute that change
	// and all slaves would comply, at least in theory.
	//
	// I think we can't destory the SceneNodes like this because
	// the destructor for each scene Node will destroy destroy it
	// and all it's childs by calling Ogre's destroySceneNode
	/*
	for(SceneNodeList::iterator iter = _scene_nodes.begin();
		iter != _scene_nodes.end(); ++iter)
	{
		delete *iter;
	}

	// @todo destroy movable objects
	for(MovableObjectList::iterator iter = _objects.begin();
		iter != _objects.end(); ++iter)
	{
		delete *iter;
	}
	*/

	// @fixme this crashes
	//delete _sky_sim;
	//_sky_sim = 0;

	// Can't destroy Ogre::SceneManager because it's owned by ogre::Root
	// not sure if it's the best decission but for now.

	// Root is already in the scene node list so don't double delete
	_root = 0;
}

void
vl::SceneManager::destroyScene(bool destroyEditorCamera)
{
	// @todo this is rather unefficient way to destroy the nodes
	// we should use the tree graph for these things...
	SceneNodeList nodes_to_destroy;
	nodes_to_destroy.reserve(_scene_nodes.size());
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		// Can't destroy the root object with this function
		if((destroyEditorCamera || _scene_nodes.at(i)->getName() != "editor/perspective")
			&& _scene_nodes.at(i)->getName() != "Root")
		{ nodes_to_destroy.push_back(_scene_nodes.at(i)); }
	}

	for(SceneNodeList::iterator iter = nodes_to_destroy.begin();
		iter != nodes_to_destroy.end(); ++iter)
	{ destroySceneNode(*iter); }


	// Destroy movable objects
	MovableObjectList objects;
	objects.reserve(_objects.size());
	for(MovableObjectList::iterator iter = _objects.begin(); 
		iter != _objects.end(); ++iter)
	{
		if(destroyEditorCamera || (*iter)->getName() != "editor/perspective")
		{ objects.push_back(*iter); }
	}

	for(MovableObjectList::iterator iter = objects.begin(); 
		iter != objects.end(); ++iter)
	{ destroyMovableObject(*iter); }
	

	delete _sky_sim;
	_sky_sim = 0;
	// Reset skydome also
	_sky_dome = SkyDomeInfo("");

	// Reset shadows
	_shadows = ShadowInfo();

	// Reset ogre
	// For some reason deserialize is not properly called after destroying
	// the Scene so we need to reset these here.
	if(_ogre_sm)
	{
		_ogre_sm->setSkyDome(false, "");
		_ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
	}
}

void
vl::SceneManager::destroyDynamicObjects(void)
{
	std::clog << "vl::SceneManager::removeDynamicObjects" << std::endl;
	SceneNodeList nodes_to_destroy;
	for(SceneNodeList::iterator iter = _scene_nodes.begin();
		iter != _scene_nodes.end(); ++iter)
	{
		if((*iter)->isDynamic())
		{ nodes_to_destroy.push_back(*iter); }
	}

	std::clog << "Destorying " << nodes_to_destroy.size() << " SceneNodes." << std::endl;
	for(SceneNodeList::iterator iter = nodes_to_destroy.begin();
		iter != nodes_to_destroy.end(); ++iter)
	{ destroySceneNode(*iter); }


	/// @todo should destroy dynamic MovableObjects also
	MovableObjectList objs_to_destroy;
	for(MovableObjectList::iterator iter = _objects.begin();
		iter != _objects.end(); ++iter)
	{
		// Can't destroy the root object with this function
		if((*iter)->isDynamic())
		{ objs_to_destroy.push_back(*iter); }
	}

	std::clog << "Destorying " << nodes_to_destroy.size() << " MovableObjects." << std::endl;
	for(MovableObjectList::iterator iter = objs_to_destroy.begin();
		iter != objs_to_destroy.end(); ++iter)
	{ destroyMovableObject(*iter); }

}

vl::SceneNodePtr
vl::SceneManager::createSceneNode(std::string const &name)
{
	if(name.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Empty SceneNode name not allowed")); }

	vl::SceneNodePtr node = _createSceneNode(name, vl::ID_UNDEFINED);
	assert(_root);
	_root->addChild(node);
	return node;
}

vl::SceneNodePtr
vl::SceneManager::createDynamicSceneNode(std::string const &name)
{
	if(name.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Empty SceneNode name not allowed")); }

	vl::SceneNodePtr node = _createSceneNode(name, vl::ID_UNDEFINED, true);
	assert(_root);
	_root->addChild(node);
	return node;
}

vl::SceneNodePtr
vl::SceneManager::createFreeSceneNode(std::string const &name)
{
	return _createSceneNode(name, vl::ID_UNDEFINED);
}

vl::SceneNodePtr
vl::SceneManager::_createSceneNode(uint64_t id)
{
	assert(vl::ID_UNDEFINED != id);
	return _createSceneNode(std::string(), id);
}

bool
vl::SceneManager::hasSceneNode(const std::string& name) const
{
	return( getSceneNode(name) );
}

vl::SceneNodePtr
vl::SceneManager::getSceneNode(const std::string& name) const
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i);
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

vl::SceneNodePtr
vl::SceneManager::getSceneNodeID(uint64_t id) const
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i);
		if( node->getID() == id )
		{ return node; }
	}

	return 0;
}

void
vl::SceneManager::destroySceneNode(SceneNodePtr node)
{
	assert(node);
	std::clog << "vl::SceneManager::destroySceneNode : " << node->getName() << std::endl;
	
	// @todo we need to remove MovableObjects
	// Remove linking
	node->removeAllChildren();
	SceneNodePtr parent = node->getParent();
	// @todo does this move the node under Root?
	if(parent)
	{
		parent->removeChild(node);
	}
	assert(!node->getParent());

	_session->deregisterObject(node);
	assert(node->getID() == vl::ID_UNDEFINED);

	delete node;

	SceneNodeList::iterator iter = std::find(_scene_nodes.begin(), _scene_nodes.end(), node);
	_scene_nodes.erase(iter);
}

void
vl::SceneManager::destroyMovableObject(vl::MovableObjectPtr object)
{
	assert(object);
	std::clog << "vl::SceneManager::destroyMovableObject: " << object->getName() << std::endl;

	// @todo this might be problematic if the user doesn't remove
	// the linkage from SceneNode
	// because unlike SceneNode we don't have a reference from
	// MovableObject to it's parent.

	_session->deregisterObject(object);
	assert(object->getID() == vl::ID_UNDEFINED);

	delete object;

	MovableObjectList::iterator iter = std::find(_objects.begin(), _objects.end(), object);
	_objects.erase(iter);
}

/// --------------------- SceneManager Entity --------------------------------
vl::EntityPtr
vl::SceneManager::createEntity(std::string const &name, 
	std::string const &mesh_name, bool use_new_mesh_manager)
{
	NamedParamList params;
	params["mesh"] = mesh_name;
	if(use_new_mesh_manager)
	{
		params["use_new_mesh_manager"] = "true";
	}
	
	return static_cast<EntityPtr>(createMovableObject(OBJ_ENTITY, name, false, params));
}

vl::EntityPtr
vl::SceneManager::createDynamicEntity(std::string const &name, 
	std::string const &mesh_name)
{
	NamedParamList params;
	params["mesh"] = mesh_name;
	
	return static_cast<EntityPtr>(createMovableObject(OBJ_ENTITY, name, true, params));
}

vl::EntityPtr
vl::SceneManager::createDynamicEntity(std::string const &name, 
	std::string const &mesh_name, bool use_new_mesh_manager)
{
	NamedParamList params;
	params["mesh"] = mesh_name;
	if(use_new_mesh_manager)
	{
		params["use_new_mesh_manager"] = "true";
	}
	
	return static_cast<EntityPtr>(createMovableObject(OBJ_ENTITY, name, true, params));
}


bool 
vl::SceneManager::hasEntity( std::string const &name ) const
{
	return hasMovableObject(OBJ_ENTITY, name);
}

vl::EntityPtr 
vl::SceneManager::getEntity( std::string const &name ) const
{
	return static_cast<EntityPtr>( getMovableObject(OBJ_ENTITY, name) );
}

/// --------------------- SceneManager Light --------------------------------
vl::LightPtr
vl::SceneManager::createLight(std::string const &name)
{
	return static_cast<LightPtr>(createMovableObject(OBJ_LIGHT, name));
}

vl::LightPtr
vl::SceneManager::createDynamicLight(std::string const &name)
{
	return static_cast<LightPtr>(createMovableObject(OBJ_LIGHT, name, true));
}

bool 
vl::SceneManager::hasLight(std::string const &name) const
{
	return hasMovableObject(OBJ_LIGHT, name);
}

vl::LightPtr 
vl::SceneManager::getLight(std::string const &name) const
{
	return static_cast<LightPtr>(getMovableObject(OBJ_LIGHT, name));
}

/// --------------------- SceneManager Camera --------------------------------
vl::CameraPtr 
vl::SceneManager::createCamera(std::string const &name)
{
	return static_cast<CameraPtr>(createMovableObject(OBJ_CAMERA, name));
}


vl::CameraPtr
vl::SceneManager::createDynamicCamera(std::string const &name)
{
	return static_cast<CameraPtr>(createMovableObject(OBJ_CAMERA, name, true));
}

bool 
vl::SceneManager::hasCamera(std::string const &name) const
{
	return hasMovableObject(OBJ_CAMERA, name);
}

vl::CameraPtr 
vl::SceneManager::getCamera(std::string const &name) const
{
	return static_cast<CameraPtr>(getMovableObject(OBJ_CAMERA, name));
}

vl::MovableTextPtr
vl::SceneManager::createMovableText(std::string const &name, std::string const &text)
{
	MovableTextPtr obj = static_cast<MovableTextPtr>(createMovableObject(OBJ_MOVABLE_TEXT, name));
	if(obj)
	{ obj->setCaption(text); }

	return obj;
}

vl::MovableTextPtr
vl::SceneManager::createDynamicMovableText(std::string const &name, std::string const &text)
{
	MovableTextPtr obj = static_cast<MovableTextPtr>(createMovableObject(OBJ_MOVABLE_TEXT, name, true));
	if(obj)
	{ obj->setCaption(text); }

	return obj;
}

bool
vl::SceneManager::hasMovableText(std::string const &name) const
{
	return hasMovableObject(OBJ_MOVABLE_TEXT, name);
}

vl::MovableTextPtr
vl::SceneManager::getMovableText(std::string const &name) const
{
	return static_cast<MovableTextPtr>(getMovableObject(OBJ_MOVABLE_TEXT, name));
}

vl::RayObjectPtr
vl::SceneManager::createRayObject(std::string const &name, std::string const &material_name)
{
	vl::RayObjectPtr obj = static_cast<RayObjectPtr>(createMovableObject(OBJ_RAY_OBJECT, name));
	obj->setMaterial(material_name);

	return obj;
}

vl::RayObjectPtr
vl::SceneManager::createDynamicRayObject(std::string const &name, std::string const &material_name)
{
	vl::RayObjectPtr obj = static_cast<RayObjectPtr>(createMovableObject(OBJ_RAY_OBJECT, name, true));
	obj->setMaterial(material_name);

	return obj;
}

bool
vl::SceneManager::hasRayObject(std::string const &name) const
{
	return hasMovableObject(OBJ_RAY_OBJECT, name);
}

vl::RayObjectPtr
vl::SceneManager::getRayObject(std::string const &name) const
{
	return static_cast<RayObjectPtr>(getMovableObject(OBJ_RAY_OBJECT, name));
}

/// ------------------ SceneManager MovableObject ----------------------------
vl::MovableObjectPtr 
vl::SceneManager::createMovableObject(std::string const &type_name, std::string const &name, vl::NamedParamList const &params)
{
	return createMovableObject(getMovableObjectType(type_name), name, false, params);
}

vl::MovableObjectPtr
vl::SceneManager::createMovableObject(vl::OBJ_TYPE type, std::string const &name, bool dynamic, vl::NamedParamList const &params)
{
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasMovableObject(type, name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	MovableObjectPtr obj = 0;
	
	switch(type)
	{
	case vl::OBJ_ENTITY:
		obj = _createEntity(name, params, dynamic);
		break;
	case vl::OBJ_LIGHT:
		obj = _createLight(name, params, dynamic);
		break;
	case vl::OBJ_CAMERA:
		obj = _createCamera(name, params, dynamic);
		break;
	case vl::OBJ_MOVABLE_TEXT:
		obj = _createMovableText(name, params, dynamic);
		break;
	case vl::OBJ_RAY_OBJECT:
		obj = _createRayObject(name, params, dynamic);
		break;
	default:
		std::cout << vl::CRITICAL << "Object type : " << type << " not a movable object." << std::endl;
		break;
	}

	_session->registerObject(obj, type, vl::ID_UNDEFINED);
	assert( obj->getID() != vl::ID_UNDEFINED );
	_objects.push_back(obj);

	return obj;
}

vl::MovableObjectPtr 
vl::SceneManager::_createMovableObject(vl::OBJ_TYPE type, uint64_t id)
{
	if( vl::ID_UNDEFINED == id )
	{ BOOST_THROW_EXCEPTION( vl::invalid_id() ); }

	MovableObjectPtr obj = 0;
	switch(type)
	{
	case OBJ_ENTITY:
		obj = new Entity(this);
		break;
	case OBJ_CAMERA:
		obj = new Camera(this);
		break;
	case OBJ_LIGHT:
		obj = new Light(this);
		break;
	case OBJ_MOVABLE_TEXT:
		obj = new MovableText(this);
		break;
	case vl::OBJ_RAY_OBJECT:
		obj = new RayObject(this);
		break;
	default :
		std::cout << vl::CRITICAL << "MovableObject type not recognised." << std::endl;
	}

	if(!obj)
	{
		BOOST_THROW_EXCEPTION(vl::invalid_param());
	}

	_session->registerObject(obj, type, id);
	assert( obj->getID() != vl::ID_UNDEFINED );
	_objects.push_back(obj);

	return obj;
}

vl::MovableObjectPtr
vl::SceneManager::_createMovableObject(std::string const &type, uint64_t id)
{
	return _createMovableObject(getMovableObjectType(type), id);
}

vl::MovableObjectPtr 
vl::SceneManager::getMovableObjectID(uint64_t id) const
{
	for( size_t i = 0; i < _objects.size(); ++i )
	{
		MovableObjectPtr obj = _objects.at(i);
		if( obj->getID() == id )
		{ return obj; }
	}

	return 0;
}

vl::MovableObjectPtr
vl::SceneManager::getMovableObject(std::string const &type_name, 
		std::string const &name) const
{
	// @todo replace to_lower with the use of case insensitive comparison
	std::string tname(type_name);
	vl::to_lower(tname);
	for( MovableObjectList::const_iterator iter = _objects.begin(); iter != _objects.end(); ++iter )
	{
		std::string it_type_name((*iter)->getTypeName());
		vl::to_lower(it_type_name);
		if( it_type_name == tname && (*iter)->getName() == name )
		{ return *iter; }
	}

	return 0;
}

/// @todo should use a faster search without string comparison
vl::MovableObjectPtr
vl::SceneManager::getMovableObject(vl::OBJ_TYPE type, std::string const &name) const
{
	return getMovableObject(getMovableObjectTypeName(type), name);
}

vl::OBJ_TYPE
vl::SceneManager::getMovableObjectType(std::string const &type) const
{
	// @todo lower case letters
	std::string type_name(type);
	vl::to_lower(type_name);
	if( type_name == "entity" )
		return OBJ_ENTITY;
	else if( type_name == "camera" )
		return OBJ_CAMERA;
	else if( type_name == "light" )
		return OBJ_LIGHT;
	else if( type_name == "movable_text" )
		return OBJ_MOVABLE_TEXT;
	else if( type_name == "ray_object" )
		return OBJ_RAY_OBJECT;
	else
		return OBJ_INVALID;
}

std::string
vl::SceneManager::getMovableObjectTypeName(vl::OBJ_TYPE type) const
{
	switch(type)
	{
	case OBJ_ENTITY:
		return "entity";
	case OBJ_CAMERA:
		return "camera";
	case OBJ_LIGHT:
		return "light";
	case OBJ_MOVABLE_TEXT:
		return "movable_text";
	case OBJ_RAY_OBJECT:
		return "ray_object";
	default:
		return "";
	}
}

bool
vl::SceneManager::hasMovableObject(vl::OBJ_TYPE type, std::string const &name) const
{
	return( getMovableObject(type, name));
}

bool 
vl::SceneManager::hasMovableObject(std::string const &type_name, std::string const &name) const
{
	return( getMovableObject(type_name, name) );
}

vl::CameraList
vl::SceneManager::getCameraList(void) const
{
	CameraList cam_list;
	for(MovableObjectList::const_iterator iter = _objects.begin(); 
		iter != _objects.end(); ++iter)
	{
		std::string t_name((*iter)->getTypeName());
		vl::to_lower(t_name);
		if(t_name == "camera")
		{ cam_list.push_back((CameraPtr)*iter); }
	}

	return cam_list;
}

/// --------------------- Scene parameters -----------------------------------
void 
vl::SceneManager::setSkyDome(SkyDomeInfo const &dome)
{
	update_variable(_sky_dome, dome, DIRTY_SKY);
}

void 
vl::SceneManager::setSkyInfo(vl::SkyInfo const &sky)
{
	update_variable(_sky, sky, DIRTY_SKY);
}

void
vl::SceneManager::setFog(FogInfo const &fog)
{
	update_variable(_fog, fog, DIRTY_FOG);
}

void 
vl::SceneManager::setAmbientLight( Ogre::ColourValue const &colour )
{
	update_variable(_ambient_light, colour, DIRTY_AMBIENT_LIGHT);
}

void
vl::SceneManager::setBackgroundColour(Ogre::ColourValue const &colour)
{
	update_variable(_background_colour, colour, DIRTY_BACKGROUND_COLOUR);
}

void 
vl::SceneManager::setShadowInfo(ShadowInfo const &info)
{
	update_variable(_shadows, info, DIRTY_SHADOW_INFO);
}

void
vl::SceneManager::reloadScene( void )
{
	std::cerr << "Should reload the scene now." << std::endl;
	setDirty( DIRTY_RELOAD_SCENE );
	_scene_version++;
}

void 
vl::SceneManager::printBoundingBoxes(void)
{
	assert( _ogre_sm );

	Ogre::SceneManager::MovableObjectIterator iter = _ogre_sm->getMovableObjectIterator("Entity");
	while( iter.hasMoreElements() )
	{
		Ogre::AxisAlignedBox const &box = iter.current()->second->getBoundingBox();
		std::clog << "Object " << iter.current()->second->getName() << " : bounding box " 
			<< box << " : size = " << box.getSize() << std::endl;
		Ogre::Vector3 diff =box.getMaximum() - box.getMinimum();
		if( diff.x < 0 || diff.y < 0 || diff.z < 0 )
		{
			std::clog << "PROBLEM : Incorrect BoundingBox in object " << iter.current()->second->getName()
				<< std::endl;
		}
		iter.moveNext();
	}
	std::clog << std::endl;
}

void
vl::SceneManager::showBoundingBoxes(bool show)
{
	SceneNodeList::iterator iter = _scene_nodes.begin();
	SceneNodeList::iterator eiter = _scene_nodes.end();
	for(;iter != eiter; ++iter)
	{
		(*iter)->setShowBoundingBox(show);
	}
}

void
vl::SceneManager::showDebugDisplays(bool show)
{
	SceneNodeList::iterator iter = _scene_nodes.begin();
	SceneNodeList::iterator eiter = _scene_nodes.end();
	for(;iter != eiter; ++iter)
	{
		(*iter)->setShowDebugDisplay(show);
	}
}

void
vl::SceneManager::showAxes(bool show)
{
	SceneNodeList::iterator iter = _scene_nodes.begin();
	SceneNodeList::iterator eiter = _scene_nodes.end();
	for(;iter != eiter; ++iter)
	{
		(*iter)->setShowAxes(show);
	}
}


/// ------------------------ SceneManager Selection --------------------------
void
vl::SceneManager::addToSelection(vl::SceneNodePtr node)
{
	if( !isInSelection(node) )
	{
		_selection.push_back(node);
		node->setShowBoundingBox(true);
	}
}

void
vl::SceneManager::removeFromSelection( vl::SceneNodePtr node )
{
	SceneNodeList::iterator iter
		= std::find(_selection.begin(), _selection.end(), node);
	if( iter != _selection.end() )
	{
		_selection.erase(iter);
		node->setShowBoundingBox(false);
	}
}

void
vl::SceneManager::setActiveObject(vl::SceneNodePtr node)
{
	_active_object = node;
}

void
vl::SceneManager::mapCollisionBarriers(void)
{
	std::map<std::string, SceneNodePtr> cbs;
	for(SceneNodeList::iterator iter = _scene_nodes.begin(); iter != _scene_nodes.end(); ++iter)
	{
		std::string const &cb_name = (*iter)->getName();
		size_t n = cb_name.find("cb_");
		if(n != std::string::npos)
		{
			std::string name = cb_name.substr(0, n) + cb_name.substr(n+3);
			cbs[name] = *iter;
		}
	}

	for(std::map<std::string, SceneNodePtr>::iterator cb_iter = cbs.begin();
		cb_iter != cbs.end(); ++cb_iter)
	{
		std::string const &name = cb_iter->first;
		bool found = false;
		for(SceneNodeList::iterator iter = _scene_nodes.begin(); iter != _scene_nodes.end(); ++iter)
		{
			if((*iter)->getName() == name)
			{
				_mapped_nodes[cb_iter->second] = *iter;
				found = true;
				break;
			}
		}

		if(!found)
		{
			std::cout << "Collision barrier with name: " << cb_iter->second->getName() 
				<< " didn't find visual object " << name << std::endl;
		}
	}
}

void
vl::SceneManager::_step(vl::time const &t)
{
	// Copy transformations for automatically mapped objects
	for(std::map<SceneNode *, SceneNode *>::iterator iter = _mapped_nodes.begin();
		iter != _mapped_nodes.end(); ++iter)
	{
		iter->second->setWorldTransform(iter->first->getWorldTransform());
	}
}

void
vl::SceneManager::_notifyFrameStart(void)
{
	for(MovableObjectList::iterator iter = _objects.begin(); iter != _objects.end(); ++iter)
	{
		(*iter)->_notifyFrameStart();
	}
}

void
vl::SceneManager::_notifyFrameEnd(void)
{
	for(MovableObjectList::iterator iter = _objects.begin(); iter != _objects.end(); ++iter)
	{
		(*iter)->_notifyFrameEnd();
	}
}

void 
vl::SceneManager::hideSceneNodes(std::string const &pattern, bool cascade, bool caseInsensitive)
{
	std::string str(pattern);
	if(caseInsensitive)
	{
		vl::to_lower(str);
	}

	std::string::size_type pos = str.find('*');
	bool asteriks = false;
	if( pos != std::string::npos )
	{ asteriks = true; }

	std::string find_name = str.substr(0, pos);

	SceneNodeList::iterator iter;
	for( iter = _scene_nodes.begin(); iter != _scene_nodes.end(); ++iter )
	{
		std::string name = (*iter)->getName();
		if(caseInsensitive)
		{ vl::to_lower(name); }
		if(asteriks)
		{ name = name.substr(0, pos); }
		
		if(find_name == name)
		{ (*iter)->setVisibility(false, cascade); }
	}
}

bool
vl::SceneManager::isInSelection(SceneNode const *node) const
{
	SceneNodeList::const_iterator iter;
	for( iter = _selection.begin(); iter != _selection.end(); ++iter )
	{
		if( *iter == node )
		{ return true; }
	}

	return false;
}

void 
vl::SceneManager::clearSelection(void)
{
	for(SceneNodeList::iterator iter = _selection.begin(); iter != _selection.end(); ++iter )
	{
		(*iter)->setShowBoundingBox(false);
	}

	_selection.clear();
}

/// ------------------------------- Private ----------------------------------
void 
vl::SceneManager::recaluclateDirties(void)
{
	if(_shadows.isDirty())
	{
		setDirty(DIRTY_SHADOW_INFO);
		_shadows.clearDirty();
	}

	if(_sky.isDirty())
	{
		setDirty(DIRTY_SKY);
		_sky.clearDirty();
	}
}

void
vl::SceneManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg << _scene_version;
	}

	if( dirtyBits & DIRTY_SKY )
	{
		msg << _sky << _sky_dome;
	}

	if( dirtyBits & DIRTY_FOG )
	{
		msg << _fog;
	}

	if( dirtyBits & DIRTY_AMBIENT_LIGHT )
	{
		msg << _ambient_light;
	}

	if(dirtyBits & DIRTY_BACKGROUND_COLOUR)
	{
		msg << _background_colour;
	}

	if(dirtyBits & DIRTY_SHADOW_INFO)
	{
		msg << _shadows;
	}
}

void
vl::SceneManager::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	assert(_ogre_sm);

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg >> _scene_version;
	}

	if( dirtyBits & DIRTY_SKY )
	{
		msg >> _sky >> _sky_dome;

		assert( _ogre_sm );

		// @todo handle updated sky and switch of sky dome
		std::map<std::string, vl::SkySettings>::iterator iter 
				= _sky_presets.find(_sky.getPreset());
		if( iter != _sky_presets.end() )
		{
			std::clog << "Should enable SkyX with " << _sky.getPreset() << " preset." << std::endl;
			if(_isSkyEnabled())
			{
				_changeSkyPreset(iter->second);
			}
			else
			{
				_initialiseSky(iter->second);
			}
		}
		else
		{
			if( _sky_dome.empty() )
			{ _ogre_sm->setSkyDome(false, ""); }
			else
			{
				_ogre_sm->setSkyDome( true, _sky_dome.material_name,
					_sky_dome.curvature, _sky_dome.tiling, _sky_dome.distance,
					_sky_dome.draw_first, _sky_dome.orientation, 
					_sky_dome.xsegments, _sky_dome.ysegments,
					_sky_dome.ysegments_keep );
			}
		}
	}

	if( dirtyBits & DIRTY_FOG )
	{
		msg >> _fog;
		if( _ogre_sm )
		{
			_ogre_sm->setFog( getOgreFogMode(_fog.mode), _fog.colour_diffuse, 
				_fog.exp_density, _fog.linear_start, _fog.linear_end );
		}
	}

	if( dirtyBits & DIRTY_AMBIENT_LIGHT )
	{
		msg >> _ambient_light;
		if( _ogre_sm )
		{
			_ogre_sm->setAmbientLight(_ambient_light);
		}
	}

	if(dirtyBits & DIRTY_BACKGROUND_COLOUR)
	{
		msg >> _background_colour;
		if( _ogre_sm )
		{
			// @todo can not change background colour without access to Viewports
		}
	}

	if(dirtyBits & DIRTY_SHADOW_INFO)
	{
		msg >> _shadows;

		/// ideally most of these should be configurable from config file
		/// because they are really performance intense, can look pretty crappy
		/// and most of them need to be static during the simulation
		if(_shadows.isEnabled())
		{
			_ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
			//_ogre_sm->setShadowColour(_shadows.getColour());
			Ogre::ShadowCameraSetupPtr cam_setup;

			if( _shadows.getCamera() == "planeoptimal" )
			{
				std::clog << "Trying to use PlaneOptimal shadow camera" 
					<< " but it's not implemented so using the previous setting." << std::endl;
				/// @todo needs a plane of interest
				// cam_setup.bind(new Ogre::PlaneOptimalShadowCameraSetup());
			}
			else if( _shadows.getCamera() == "lispsm" )
			{
				std::clog << "Using LiSPSM shadow camera" << std::endl;
				Ogre::LiSPSMShadowCameraSetup *lispsm = new Ogre::LiSPSMShadowCameraSetup();
				//setOptimalAdjustFactor
				//setCameraLightDirectionThreshold
				cam_setup.bind(lispsm);
			}
			else if( _shadows.getCamera() == "pssm" )
			{
				std::clog << "Using PSSM shadow camera" << std::endl;
				Ogre::PSSMShadowCameraSetup *pssm = new Ogre::PSSMShadowCameraSetup();
				size_t const n_maps = 3;
				pssm->calculateSplitPoints(n_maps, 0.1, 100);
				cam_setup.bind(pssm);

				// Necessary for PSSM for other types we should reset this
				_ogre_sm->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, n_maps);
				_ogre_sm->setShadowTextureCountPerLightType(Ogre::Light::LT_SPOTLIGHT, n_maps);
				// Hard coded max lights for shadows, 4 at the moment
				_ogre_sm->setShadowTextureCount(4*n_maps);
			}
			else if( _shadows.getCamera() == "focused" )
			{
				// Focused shadow camera causes artifacts when ran on multiple screen.
				std::clog << "Using Focused shadow camera" << std::endl;
				cam_setup.bind(new Ogre::FocusedShadowCameraSetup());
			}
			else // if( _shadows.getCamera() == "default" )
			{
				std::clog << "Using Default shadow camera" << std::endl;
				cam_setup.bind(new Ogre::DefaultShadowCameraSetup());
			}

			if( !cam_setup.isNull() )
			{ _ogre_sm->setShadowCameraSetup(cam_setup); }
		}
		else
		{ _ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_NONE); }

		/// @todo these should only be set the first time or after they
		/// have changed.

		/// These can not be moved to SceneManager at least not yet
		/// because they need the RenderSystem capabilities.
		/// @todo this should be user configurable (if the hardware supports it)
		/// @todo the number of textures (four at the moment) should be user configurable
		if(Ogre::Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_HWRENDER_TO_TEXTURE))
		{
			std::cout << "Using " << _shadows.getTextureSize() << " x " 
				<< _shadows.getTextureSize() << " shadow textures." << std::endl;
			_ogre_sm->setShadowTextureSettings(_shadows.getTextureSize(), 4);
		}
		else
		{
			/// @todo this doesn't work on Windows with size < (512,512)
			/// should check the window size and select the largest
			/// possible shadow texture based on that.
			std::cout << "Hardware does not support offscreen buffers. " <<
				" So using 512 x 512 shadow textures for compatibility." << std::endl;
			_ogre_sm->setShadowTextureSettings(512, 4);
		}

		// Disable shadow textures for POINT lights as they are not supported
		// All textures are available for spot and directional lights.
		// @todo
		// POINT light shadows are supported by Ogre, we just don't know the
		// parameters we should use with those. Also they can cause problems
		// because they use the camera direction (like directional lights)
		// so synchronisation issues with multiple viewports might be a problem.
		_ogre_sm->setShadowTextureCountPerLightType(Ogre::Light::LT_POINT, 0);

		/// @todo for self shadowing we need to use custom shaders
		/// @todo make configurable
		_ogre_sm->setShadowTextureSelfShadow(_shadows.isShelfShadowEnabled());
		/// Hard coded floating point texture, needs current hardware
		/// provides much better depth map
		/// @todo these are not runtime configurable variables they should
		/// be set elsewhere
		_ogre_sm->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
		_ogre_sm->setShadowTextureCasterMaterial(_shadows.getShadowCasterMaterial());

		/// For fixed functionality texture shadows to work this must be set
		/// @todo make configurable
		/// @todo add support for it in Shaders (just clip the shadow)
		_ogre_sm->setShadowFarDistance(_shadows.getMaxDistance());
		_ogre_sm->setShadowDirectionalLightExtrusionDistance(_shadows.getDirLightExtrusionDistance());

		// Dir light texture offset gives better looking shadows, but
		// the catch 22 is that it breaks synchronisation of multiple screens
		// because it implicitly uses different camera setups of those
		// screens.
		// If necessary we should implement our own shadow camera type that
		// would use the Player position and head data, but not the
		// screen specific camera information.
		_ogre_sm->setShadowDirLightTextureOffset(0);
	}
}

vl::SceneNodePtr 
vl::SceneManager::_createSceneNode(std::string const &name, uint64_t id, bool dynamic)
{
	assert( !name.empty() || vl::ID_UNDEFINED != id );

	// Check that no two nodes have the same name
	if( !name.empty() && hasSceneNode(name) )
	{
		// TODO is this the right exception?
		std::string msg("SceneNode with that name already exists");
		BOOST_THROW_EXCEPTION( vl::duplicate() << vl::desc(msg) << vl::name(name) );
	}
	assert( _session );

	SceneNodePtr node = new SceneNode(name, this, dynamic);

	_session->registerObject( node, OBJ_SCENE_NODE, id );
	assert( node->getID() != vl::ID_UNDEFINED );
	_scene_nodes.push_back( node );

	return node;
}

vl::MovableObjectPtr
vl::SceneManager::_createEntity(std::string const &name, vl::NamedParamList const &params, bool dynamic)
{
	std::string mesh_name;
	NamedParamList::const_iterator iter = params.find("mesh");
	if(iter != params.end())
	{ mesh_name = iter->second; }

	bool use_new_manager = false;
	iter = params.find("use_new_mesh_manager");
	if(iter != params.end())
	{
		use_new_manager = vl::from_string<bool>(iter->second);
	}

	if(!mesh_name.empty())
	{
		return new Entity(name, mesh_name, this, dynamic, use_new_manager);
	}
	else
	{
		BOOST_THROW_EXCEPTION(vl::invalid_param() << vl::desc("Mesh name can not be empty"));
	}
}

vl::MovableObjectPtr
vl::SceneManager::_createLight(std::string const &name, vl::NamedParamList const &params, bool dynamic)
{
	std::clog << "vl::SceneManager::_createLight" << std::endl;
	// Does not accept any params for now
	return new Light(name, this, dynamic);
}

vl::MovableObjectPtr
vl::SceneManager::_createCamera(std::string const &name, vl::NamedParamList const &params, bool dynamic)
{
	// Does not accept any params for now
	return new Camera(name, this, dynamic);
}

vl::MovableObjectPtr
vl::SceneManager::_createMovableText(std::string const &name, vl::NamedParamList const &params, bool dynamic)
{
	// Does not accept any params for now
	return new MovableText(name, this, dynamic);
}


vl::MovableObjectPtr
vl::SceneManager::_createRayObject(std::string const &name, vl::NamedParamList const &params, bool dynamic)
{
	// Does not accept any params for now
	return new RayObject(name, this, dynamic);
}

void
vl::SceneManager::_initialiseSky(vl::SkySettings const &preset)
{
	if(_sky_sim)
	{ return; }

	_sky_sim = new vl::SkySkyX(this);
	//_sky_sim = new vl::SkyCaelum(this);

	_changeSkyPreset(preset);
}

void
vl::SceneManager::_changeSkyPreset(vl::SkySettings const &preset)
{
	std::clog << "vl::SceneManager::_changeSkyPreset" << std::endl;
	assert(_sky_sim);

	_sky_sim->setTimeMultiplier(preset.timeMultiplier);
	_sky_sim->setTime(preset.time.x);
	_sky_sim->setSunriseTime(preset.time.y);
	_sky_sim->setSunsetTime(preset.time.z);
	_sky_sim->setMoonPhase(preset.moonPhase);

	//_skyX->getAtmosphereManager()->setOptions(preset.atmosphereOpt);

	_sky_sim->setWindSpeed(preset.vcWindSpeed);
	//_skyX->getVCloudsManager()->setAutoupdate(preset.vcAutoupdate);

	// @todo switch from Radians to Ogre::Vector2 or Ogre::Vector3
	_sky_sim->setWindDirection(preset.vcWindDir);
	_sky_sim->setAmbientColor(preset.vcAmbientColor);
	_sky_sim->setLightResponse(preset.vcLightResponse);
	_sky_sim->setAmbientFactors(preset.vcAmbientFactors);
	_sky_sim->setWeather(preset.vcWheater);

	_sky_sim->enableVolumetricClouds(preset.volumetricClouds);

	_sky_sim->update(vl::time());
}

bool
vl::SceneManager::_isSkyEnabled(void) const
{
	return _sky_sim;
}

/// --------------------------------- Global ---------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::SceneManager const &scene)
{
	os << "SceneManager : with " << scene.getSceneNodeList().size() << " scene nodes "
		<< " and " << scene.getMovableObjectList().size() << " objects.\n"
		<< "With a selection of " << scene.getSelection().size() << " numbers." << std::endl;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::SkyDomeInfo const &sky)
{
	os << "SkyDome : material " << sky.material_name
		<< " : curvature " << sky.curvature
		<< " : tiling " << sky.tiling
		<< " : distance " << sky.distance
		<< " : draw_first " << sky.draw_first
		<< " : orientation " << sky.orientation
		<< " : xsegments " << sky.xsegments
		<< " : ysegments " << sky.ysegments;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::FogInfo const &fog)
{
	os << "Fog : mode " << getFogModeAsString(fog.mode)
		<< " : diffuse colour " << fog.colour_diffuse
		<< " : density " << fog.exp_density
		<< " : start " << fog.linear_start
		<< " : end " << fog.linear_end;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::ShadowInfo const &shadows)
{
	os << "Shadows : ";
	if(shadows.isEnabled())
	{ os << "enabled "; }
	else
	{ os << "disabled "; }
	if(shadows.isShelfShadowEnabled())
	{ os << " : with shelf shadowing"; }
	else
	{ os << " : without shelf shadowing"; }
	os  << " : camera " << shadows.getCamera()
		<< " : texture size " << shadows.getTextureSize()
		<< " : max distance " << shadows.getMaxDistance()
		<< " : caster material " << shadows.getShadowCasterMaterial();

	return os;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::SkyDomeInfo const &sky)
{
	msg << sky.material_name
		<< sky.curvature
		<< sky.tiling
		<< sky.distance
		<< sky.draw_first
		<< sky.orientation
		<< sky.xsegments
		<< sky.ysegments
		<< sky.ysegments_keep;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::SkyDomeInfo &sky)
{
	msg >> sky.material_name
		>> sky.curvature
		>> sky.tiling
		>> sky.distance
		>> sky.draw_first
		>> sky.orientation
		>> sky.xsegments
		>> sky.ysegments
		>> sky.ysegments_keep;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::FogInfo const &fog)
{
	msg << fog.mode
		<< fog.colour_diffuse
		<< fog.exp_density
		<< fog.linear_start
		<< fog.linear_end;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::FogInfo &fog)
{
	msg >> fog.mode
		>> fog.colour_diffuse
		>> fog.exp_density
		>> fog.linear_start
		>> fog.linear_end;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::ShadowInfo const &shadows)
{
	msg << shadows.isEnabled() << shadows.getCamera() << shadows.getTextureSize()
		<< shadows.getShadowCasterMaterial() << shadows.getMaxDistance()
		<< shadows.isShelfShadowEnabled() << shadows.getDirLightExtrusionDistance();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::ShadowInfo &shadows)
{
	std::string camera, caster_material;
	bool enabled, shelf_shadow;
	int texture_size;
	vl::scalar dist, dir_extrusion_dist;

	msg >> enabled >> camera >> texture_size
		>> caster_material >> dist
		>> shelf_shadow >> dir_extrusion_dist;

	shadows.setCamera(camera);
	shadows.setTextureSize(texture_size);
	shadows.setEnabled(enabled);
	shadows.setShadowCasterMaterial(caster_material);
	shadows.setMaxDistance(dist);
	shadows.setShelfShadowEnabled(shelf_shadow);
	shadows.setDirLightExtrusionDistance(dir_extrusion_dist);

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::SkyInfo const &sky)
{
	msg << sky.getPreset();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::SkyInfo &sky)
{
	std::string str;
	msg >> str;
	sky.setPreset(str);

	return msg;
}
