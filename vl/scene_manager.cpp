/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.cpp
 */

#include "scene_manager.hpp"

#include "scene_node.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "movable_text.hpp"
#include "ray_object.hpp"

/// Necessary for creating PREFABS
#include "mesh_manager.hpp"

/// Necessary for better shadow camera
#include <OGRE/OgreShadowCameraSetupLiSPSM.h>
#include <OGRE/OgreShadowCameraSetupPlaneOptimal.h>
#include "logger.hpp"

namespace
{

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

Ogre::ShadowTechnique getOgreShadowTechnique(vl::ShadowTechnique t)
{
	switch(t)
	{
	case vl::SHADOWTYPE_NONE:
		return Ogre::SHADOWTYPE_NONE;
	case vl::SHADOWTYPE_TEXTURE_MODULATIVE:
		return Ogre::SHADOWTYPE_TEXTURE_MODULATIVE;
	case vl::SHADOWTYPE_TEXTURE_ADDITIVE:
		return Ogre::SHADOWTYPE_TEXTURE_ADDITIVE;
	case vl::SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED:
		return Ogre::SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED;
	case vl::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED:
		return Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED;
	case vl::SHADOWTYPE_STENCIL_MODULATIVE:
		return Ogre::SHADOWTYPE_STENCIL_MODULATIVE;
	case vl::SHADOWTYPE_STENCIL_ADDITIVE:
		return Ogre::SHADOWTYPE_STENCIL_ADDITIVE;
	default:
		return Ogre::SHADOWTYPE_NONE;
	}
}

}

std::string vl::getShadowTechniqueAsString(ShadowTechnique tech)
{
	switch(tech)
	{
	case SHADOWTYPE_TEXTURE_MODULATIVE:
		return "texture_modulative";
	case SHADOWTYPE_STENCIL_MODULATIVE:
		return "stencil_modulative";
	case SHADOWTYPE_TEXTURE_ADDITIVE:
		return "texture_additive";
	case SHADOWTYPE_STENCIL_ADDITIVE:
		return "stencil_additive";
	case SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED:
		return "texture_modulative_integrated";
	case SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED:
		return "texture_additive_integrated";
	case SHADOWTYPE_NONE:
		return "none";
	default :
		return "UNKNOWN";
	}
}

vl::ShadowTechnique vl::getShadowTechniqueFromString(std::string const &tech)
{
	std::string str(tech);
	vl::to_lower(str);
	if( str == "texture_modulative" || str == "texture" )
	{ return SHADOWTYPE_TEXTURE_MODULATIVE; }
	else if( str == "stencil_modulative" || str == "stencil" )
	{ return SHADOWTYPE_STENCIL_MODULATIVE; }
	else if( str == "texture_modulative_integrated" )
	{ return SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED; }
	else if( str == "texture_additive" )
	{ return SHADOWTYPE_TEXTURE_ADDITIVE; }
	else if( str == "texture_additive_integrated" )
	{ return SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED; }
	else if( str == "stencil_additive" )
	{ return SHADOWTYPE_STENCIL_ADDITIVE; }
	else if( str == "none" )
	{ return SHADOWTYPE_NONE; }
	else
	{ return SHADOWTYPE_NOT_VALID; }
}

/// ------------------------------ ShadowInfo --------------------------------
vl::ShadowInfo::ShadowInfo(std::string const &tech, Ogre::ColourValue const &col, std::string const &cam)
	: _technique(SHADOWTYPE_NONE)
	, _colour(col)
	, _camera(cam)
	, _enabled(false)
	, _dirty(true)
{
	setShadowTechnique(tech);
}

void
vl::ShadowInfo::disable(void)
{
	if(_enabled)
	{
		_setDirty();
		_enabled = false;
	}
}

void
vl::ShadowInfo::enable(void)
{
	if(!_enabled || _technique == SHADOWTYPE_NONE)
	{
		_setDirty();
		_enabled = true;
		// Only change technique if we have no shadows
		if(_technique == SHADOWTYPE_NONE)
		{ _technique = SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED; }
	}
}

void
vl::ShadowInfo::setShadowTechnique(std::string const &tech)
{
	ShadowTechnique technique = getShadowTechniqueFromString(tech);
	if(technique != SHADOWTYPE_NOT_VALID && technique != _technique)
	{
		_setDirty();
		_technique = technique;
	}
}

std::string
vl::ShadowInfo::getShadowTechniqueName(void) const
{
	return getShadowTechniqueAsString(_technique);
}

void 
vl::ShadowInfo::setShadowTechnique(ShadowTechnique tech)
{
	if(tech != _technique)
	{
		_setDirty();
		_technique = tech;
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
vl::ShadowInfo::setColour(Ogre::ColourValue const &col)
{
	if(col != _colour)
	{
		_setDirty();
		_colour = col;
	}
}

std::string const RAY_PARENT_NAME = "RecordedRayParent";

/// ------------------------------ SceneManager ------------------------------
/// Public
/// Master constructor
vl::SceneManager::SceneManager(vl::Session *session, vl::MeshManagerRefPtr mesh_man)
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _recorded_rays_show(false)
	, _ray_parent(0)
	, _session(session)
	, _mesh_manager(mesh_man)
	, _ogre_sm(0)
{
	std::cout << vl::TRACE << "vl::SceneManager::SceneManager" << std::endl;

	_session->registerObject( this, OBJ_SCENE_MANAGER);
	_root = createFreeSceneNode("Root");
	// @todo ray parent should star hidden
	_ray_parent = _root->createChildSceneNode(RAY_PARENT_NAME);
}

/// Renderer constructor
vl::SceneManager::SceneManager(vl::Session *session, uint64_t id, Ogre::SceneManager *native, vl::MeshManagerRefPtr mesh_man)
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _recorded_rays_show(false)
	, _ray_parent(0)
	, _session(session)
	, _mesh_manager(mesh_man)
	, _ogre_sm(native)
{
	std::cout << vl::TRACE << "vl::SceneManager::SceneManager" << std::endl;

	assert(_session);
	assert(id != vl::ID_UNDEFINED );
	assert(_ogre_sm);

	_session->registerObject( this, OBJ_SCENE_MANAGER, id );
}

vl::SceneManager::~SceneManager( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{ delete _scene_nodes.at(i); }

	_scene_nodes.clear();
}

vl::SceneNodePtr
vl::SceneManager::createSceneNode(std::string const &name)
{
	vl::SceneNodePtr node = _createSceneNode(name, vl::ID_UNDEFINED);
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

/// --------------------- SceneManager Entity --------------------------------
vl::EntityPtr
vl::SceneManager::createEntity(std::string const &name, vl::PREFAB type)
{
	/// @todo change to use createMovableObject

	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	EntityPtr ent = 0;
	/// Test code for new MeshManager
	if(type == PF_PLANE)
	{
		if(!_mesh_manager)
		{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }
		
		std::string mesh_name("prefab_plane");
		if(!_mesh_manager->hasMesh(mesh_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			_mesh_manager->createPlane(mesh_name, 20, 20);
		}

		ent = new Entity(name, mesh_name, this, true);
	}
	else if(type == PF_CUBE)
	{
		if(!_mesh_manager)
		{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }
		
		std::string mesh_name("prefab_cube");
		if(!_mesh_manager->hasMesh(mesh_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			_mesh_manager->createCube(mesh_name);
		}

		ent = new Entity(name, mesh_name, this, true);
	}
	/// Old system for other PREFABS
	else
	{
		ent = new Entity(name, type, this);
	}

	_session->registerObject( ent, OBJ_ENTITY, vl::ID_UNDEFINED );
	assert( ent->getID() != vl::ID_UNDEFINED );
	_objects.push_back(ent);

	return ent;
}

vl::EntityPtr 
vl::SceneManager::createEntity(std::string const &name, std::string const &mesh_name)
{
	return createEntity(name, mesh_name, false);
}

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
	
	return static_cast<EntityPtr>(createMovableObject(OBJ_ENTITY, name, params));
}

vl::EntityPtr
vl::SceneManager::_createEntity(uint64_t id)
{
	return static_cast<EntityPtr>(_createMovableObject("Entity", id));
}

bool 
vl::SceneManager::hasEntity( std::string const &name ) const
{
	return hasMovableObject("Entity", name);
}

vl::EntityPtr 
vl::SceneManager::getEntity( std::string const &name ) const
{
	return static_cast<EntityPtr>( getMovableObject("Entity", name) );
}

/// --------------------- SceneManager Light --------------------------------
vl::LightPtr
vl::SceneManager::createLight(std::string const &name)
{
	return static_cast<LightPtr>(createMovableObject(OBJ_LIGHT, name));
}

vl::LightPtr
vl::SceneManager::_createLight(uint64_t id)
{
	return static_cast<LightPtr>(_createMovableObject("Light", id));
}

bool 
vl::SceneManager::hasLight(std::string const &name) const
{
	return hasMovableObject("Light", name);
}

vl::LightPtr 
vl::SceneManager::getLight(std::string const &name) const
{
	return static_cast<LightPtr>(getMovableObject("Light", name));
}

/// --------------------- SceneManager Camera --------------------------------
vl::CameraPtr 
vl::SceneManager::createCamera(std::string const &name)
{
	return static_cast<CameraPtr>(createMovableObject(OBJ_CAMERA, name));
}

vl::CameraPtr 
vl::SceneManager::_createCamera(uint64_t id)
{
	return static_cast<CameraPtr>(_createMovableObject("Camera", id));
}

bool 
vl::SceneManager::hasCamera(std::string const &name) const
{
	return hasMovableObject("Camera", name);
}

vl::CameraPtr 
vl::SceneManager::getCamera(std::string const &name) const
{
	return static_cast<CameraPtr>(getMovableObject("Camera", name));
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
vl::SceneManager::_createMovableText(uint64_t id)
{
	return static_cast<MovableTextPtr>(_createMovableObject("MovableText", id));
}

vl::MovableObjectPtr
vl::SceneManager::createRayObject(std::string const &name, std::string const &material_name)
{
	vl::RayObjectPtr obj = static_cast<RayObjectPtr>(createMovableObject(OBJ_RAY_OBJECT, name));
	obj->setMaterial(material_name);

	return obj;
}


/// ------------------ SceneManager MovableObject ----------------------------
vl::MovableObjectPtr 
vl::SceneManager::createMovableObject(std::string const &type_name, std::string const &name, vl::NamedParamList const &params)
{
	return createMovableObject(getMovableObjectType(type_name), name, params);
}

vl::MovableObjectPtr
vl::SceneManager::createMovableObject(vl::OBJ_TYPE type, std::string const &name, vl::NamedParamList const &params)
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
		obj = _createEntity(name, params);
		break;
	case vl::OBJ_LIGHT:
		obj = _createLight(name, params);
		break;
	case vl::OBJ_CAMERA:
		obj = _createCamera(name, params);
		break;
	case vl::OBJ_MOVABLE_TEXT:
		obj = _createMovableText(name, params);
		break;
	case vl::OBJ_RAY_OBJECT:
		obj = _createRayObject(name, params);
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
	for( MovableObjectList::const_iterator iter = _objects.begin(); iter != _objects.end(); ++iter )
	{
		if( (*iter)->getTypeName() == type_name && (*iter)->getName() == name )
		{ return *iter; }
	}

	return 0;
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
	return hasMovableObject(getMovableObjectTypeName(type), name);
}

bool 
vl::SceneManager::hasMovableObject(std::string const &type_name, std::string const &name) const
{
	return( getMovableObject(type_name, name) );
}


/// --------------------- Scene parameters -----------------------------------
void 
vl::SceneManager::setSkyDome(SkyDomeInfo const &dome)
{
	/// @todo add checking that the previous SkyDome is different
	setDirty(DIRTY_SKY_DOME);
	_sky_dome = dome;
}

void 
vl::SceneManager::setFog(FogInfo const &fog)
{
	/// @todo add checking that the previous fog is different
	setDirty(DIRTY_FOG);
	_fog = fog;
}

void 
vl::SceneManager::setAmbientLight( Ogre::ColourValue const &colour )
{
	if( _ambient_light != colour )
	{
		setDirty( DIRTY_AMBIENT_LIGHT );
		_ambient_light = colour;
	}
}

void
vl::SceneManager::setBackgroundColour(Ogre::ColourValue const &colour)
{
	if(_background_colour != colour)
	{
		setDirty(DIRTY_BACKGROUND_COLOUR);
		_background_colour = colour;
	}
}

void 
vl::SceneManager::setShadowInfo(ShadowInfo const &info)
{
	if( info != _shadows )
	{
		setDirty(DIRTY_SHADOW_INFO);
		_shadows = info;
	}
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
vl::SceneManager::mapCollisionBarriers(void)
{
	SceneNodeList cbs;
	for(SceneNodeList::iterator iter = _scene_nodes.begin(); iter != _scene_nodes.end(); ++iter)
	{
		if((*iter)->getName().substr(0, 3) == "cb_")
		{
			cbs.push_back(*iter);
		}
	}

	for(SceneNodeList::iterator cb_iter = cbs.begin(); cb_iter != cbs.end(); ++cb_iter)
	{
		std::string name = (*cb_iter)->getName().substr(3);
		bool found = false;
		for(SceneNodeList::iterator iter = _scene_nodes.begin(); iter != _scene_nodes.end(); ++iter)
		{
			if((*iter)->getName() == name)
			{
				_mapped_nodes[*cb_iter] = *iter;
				found = true;
				break;
			}
		}

		if(!found)
		{
			std::cout << "Collision barrier with name: " << (*cb_iter)->getName() 
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
vl::SceneManager::hideSceneNodes(std::string const &pattern, bool caseInsensitive)
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
		{ (*iter)->setVisible(false); }
	}
}

void
vl::SceneManager::addRecordedRay(vl::Transform const &t)
{
	std::stringstream ss;
	ss << "recorded_ray_" << _recorded_rays.size()-1;
	// Add a ray object
	// @todo fix the hard-coded material.
	RayObjectPtr ray = static_cast<RayObjectPtr>(createRayObject(ss.str(), "finger_sphere/red"));

	// @todo add parent for all rays
	assert(_ray_parent);
	_ray_parent->attachObject(ray);
	ray->setDynamic(false);
	ray->setPosition(t.position);
	// @todo is this negative or positive?
	ray->setDirection(t.quaternion*Ogre::Vector3::UNIT_Z);

	_recorded_rays.push_back(ray);

	// Only update slaves if the rays are visible
	// Is this necessary? Do we really need the object list in the slaves?
	// As we are already adding an object and creating the CREATE_MSG
	// then syncing the object which is done automatically by UPDATE_MSG
	/*
	if(_recorded_rays_show)
	{
		setDirty(DIRTY_RECORDED_RAYS);
	}
	*/
}

void
vl::SceneManager::showRecordedRays(bool show)
{
	if(show != _recorded_rays_show)
	{
		setDirty(DIRTY_RECORDED_RAYS);
		_recorded_rays_show = show;
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
}

void
vl::SceneManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg << _scene_version;
	}

	if( dirtyBits & DIRTY_SKY_DOME )
	{
		msg << _sky_dome;
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

	if(dirtyBits & DIRTY_RECORDED_RAYS)
	{
		msg << _recorded_rays_show;
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

	if( dirtyBits & DIRTY_SKY_DOME )
	{
		msg >> _sky_dome;
		if( _ogre_sm )
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
			_ogre_sm->setShadowTechnique( getOgreShadowTechnique(_shadows.getShadowTechnique()) );
			_ogre_sm->setShadowColour(_shadows.getColour());
			Ogre::ShadowCameraSetupPtr cam_setup;
			if( _shadows.getCamera() == "default" )
			{
				cam_setup.bind(new Ogre::DefaultShadowCameraSetup());
			}
			else if( _shadows.getCamera() == "planeoptimal" )
			{
				/// @todo needs a plane of interest
				// cam_setup.bind(new Ogre::PlaneOptimalShadowCameraSetup());
			}
			else if( _shadows.getCamera() == "lispsm" )
			{
				/// LiSPSM has creately softer shadows compared to the default one
				/// i.e. less pixelisation in the edges.
				cam_setup.bind(new Ogre::LiSPSMShadowCameraSetup());
			}

			if( !cam_setup.isNull() )
			{ _ogre_sm->setShadowCameraSetup(cam_setup); }
		}
		else
		{ _ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_NONE); }
	
		/// @todo for self shadowing we need to use custom shaders
		/// @todo make configurable
		_ogre_sm->setShadowTextureSelfShadow(true);
		/// Hard coded floating point texture, needs current hardware
		/// provides much better depth map
		/// @todo these are not runtime configurable variables they should
		/// be set elsewhere
		_ogre_sm->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
		_ogre_sm->setShadowTextureCasterMaterial("ShadowCaster");

		/// For fixed functionality texture shadows to work this must be set
		/// @todo make configurable
		/// @todo add support for it in Shaders (just clip the shadow)
		_ogre_sm->setShadowFarDistance(50);
	}

	if(dirtyBits & DIRTY_RECORDED_RAYS)
	{
		msg >> _recorded_rays_show;
		
		// Try to find the parent
		if(!_ray_parent)
		{
			for(SceneNodeList::iterator iter = _scene_nodes.begin();
				iter != _scene_nodes.end(); ++iter)
			{
				if((*iter)->getName() == RAY_PARENT_NAME)
				{
					_ray_parent = *iter;
					break;
				}
			}
		}

		// @todo show hide them
		if(_ray_parent)
		{ _ray_parent->setVisible(_recorded_rays_show); }
	}
}

vl::SceneNodePtr 
vl::SceneManager::_createSceneNode(std::string const &name, uint64_t id)
{
	assert( !name.empty() || vl::ID_UNDEFINED != id );

	// Check that no two nodes have the same name
	if( !name.empty() && hasSceneNode(name) )
	{
		// TODO is this the right exception?
		BOOST_THROW_EXCEPTION( vl::duplicate() );
	}
	assert( _session );

	SceneNodePtr node = new SceneNode( name, this );

	_session->registerObject( node, OBJ_SCENE_NODE, id );
	assert( node->getID() != vl::ID_UNDEFINED );
	_scene_nodes.push_back( node );

	return node;
}

vl::MovableObjectPtr
vl::SceneManager::_createEntity(std::string const &name, vl::NamedParamList const &params)
{
	std::string mesh_name;
	NamedParamList::const_iterator iter = params.find("mesh");
	if(iter != params.end())
	{ mesh_name = iter->second; }
	std::string prefab;
	iter = params.find("prefab");
	if(iter != params.end())
	{ prefab = iter->second; }

	bool use_new_manager = false;
	iter = params.find("use_new_mesh_manager");
	if(iter != params.end())
	{
		use_new_manager = vl::from_string<bool>(iter->second);
	}

	if(mesh_name.empty() && prefab.empty())
	{
		BOOST_THROW_EXCEPTION(vl::invalid_param() << vl::desc("Mesh name can not be empty"));
	}
	else if(!mesh_name.empty())
	{
		return new Entity(name, mesh_name, this, use_new_manager);
	}
	else // !prefab.empty()
	{
		PREFAB pf = PF_NONE;
		vl::to_lower(prefab);
		if(prefab == "plane")
		{ pf = PF_PLANE;}
		else if(prefab == "cube")
		{ pf = PF_CUBE;}
		else if(prefab == "sphere")
		{ pf = PF_SPHERE;}
		else
		{ BOOST_THROW_EXCEPTION(vl::invalid_param() << vl::desc("Mesh Prefab invalid.")); }
		
		return new Entity(name, pf, this);
	}
}

vl::MovableObjectPtr
vl::SceneManager::_createLight(std::string const &name, vl::NamedParamList const &params)
{
	// Does not accept any params for now
	return new Light(name, this);
}

vl::MovableObjectPtr
vl::SceneManager::_createCamera(std::string const &name, vl::NamedParamList const &params)
{
	// Does not accept any params for now
	return new Camera(name, this);
}

vl::MovableObjectPtr
vl::SceneManager::_createMovableText(std::string const &name, vl::NamedParamList const &params)
{
	// Does not accept any params for now
	return new MovableText(name, this);
}


vl::MovableObjectPtr
vl::SceneManager::_createRayObject(std::string const &name, vl::NamedParamList const &params)
{
	// Does not accept any params for now
	return new RayObject(name, this);
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
	os << ": technique " << shadows.getShadowTechniqueName()
		<< " : colour " << shadows.getColour()
		<< " : camera " << shadows.getCamera();

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
	msg << shadows.getShadowTechnique() << shadows.getColour() << shadows.getCamera() << shadows.isEnabled();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::ShadowInfo &shadows)
{
	ShadowTechnique tech;
	Ogre::ColourValue col;
	std::string camera;
	bool enabled;
	msg >> tech >> col >> camera >> enabled;

	shadows.setShadowTechnique(tech);
	shadows.setColour(col);
	shadows.setCamera(camera);
	if(enabled)
	{ shadows.enable(); }
	else
	{ shadows.disable(); }

	return msg;
}
