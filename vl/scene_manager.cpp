/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.cpp
 */

#include "scene_manager.hpp"

#include "scene_node.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"

/// Necessary for better shadow camera
#include <OGRE/OgreShadowCameraSetupLiSPSM.h>
#include <OGRE/OgreShadowCameraSetupPlaneOptimal.h>

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

void
vl::ShadowInfo::setShadowTechnique(std::string const &tech)
{
	std::string str(tech);
	vl::to_lower(str);
	if( str == "texture_modulative" || str == "texture" )
	{
		technique = SHADOWTYPE_TEXTURE_MODULATIVE;
	}
	else if( str == "stencil_modulative" || str == "stencil" )
	{
		technique = SHADOWTYPE_STENCIL_MODULATIVE;
	}
	else if( str == "texture_modulative_integrated" )
	{
		technique = SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED;
	}
	else if( str == "texture_additive" )
	{
		technique = SHADOWTYPE_TEXTURE_ADDITIVE;
	}
	else if( str == "texture_additive_integrated" )
	{
		technique = SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED;
	}
	else if( str == "stencil_additive" )
	{
		technique = SHADOWTYPE_STENCIL_ADDITIVE;
	}
	else if( str == "none" )
	{
		technique = SHADOWTYPE_NONE;
	}
	else
	{
		// Not a valid technique
		return;
	}
}

std::string
vl::ShadowInfo::getShadowTechnique(void) const
{
	switch(technique)
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

void
vl::ShadowInfo::disable(void)
{
	technique = SHADOWTYPE_NONE;
}

void
vl::ShadowInfo::enable(void)
{
	technique = SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED;
}

/// Public
/// Master constructor
vl::SceneManager::SceneManager(vl::Session *session)
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _session(session)
	, _ogre_sm(0)
{
	_session->registerObject( this, OBJ_SCENE_MANAGER);
	_root = createFreeSceneNode("Root");
}

/// Renderer constructor
vl::SceneManager::SceneManager( vl::Session *session, uint64_t id, Ogre::SceneManager *native )
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _session(session)
	, _ogre_sm(native)
{
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
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	EntityPtr ent = new Entity(name, type, this);
	
	_session->registerObject( ent, OBJ_ENTITY, vl::ID_UNDEFINED );
	assert( ent->getID() != vl::ID_UNDEFINED );
	_objects.push_back(ent);

	return ent;
}

vl::EntityPtr
vl::SceneManager::createEntity(std::string const &name, std::string const &mesh_name)
{
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	EntityPtr ent = new Entity(name, mesh_name, this);
	
	_session->registerObject(ent, OBJ_ENTITY, vl::ID_UNDEFINED);
	assert( ent->getID() != vl::ID_UNDEFINED );
	_objects.push_back(ent);

	return ent;
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
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	LightPtr light = new Light(name, this);
	
	_session->registerObject(light, OBJ_LIGHT, vl::ID_UNDEFINED);
	assert( light->getID() != vl::ID_UNDEFINED );
	_objects.push_back(light);

	return light;
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
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	CameraPtr cam = new Camera(name, this);
	
	_session->registerObject(cam, OBJ_CAMERA, vl::ID_UNDEFINED);
	assert( cam->getID() != vl::ID_UNDEFINED );
	_objects.push_back(cam);

	return cam;
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

/// ------------------ SceneManager MovableObject ----------------------------
vl::MovableObjectPtr
vl::SceneManager::_createMovableObject(std::string const &type, uint64_t id)
{
	if( vl::ID_UNDEFINED == id )
	{ BOOST_THROW_EXCEPTION( vl::invalid_id() ); }

	MovableObjectPtr obj = 0;
	OBJ_TYPE t;
	if( type == "Entity" )
	{ 
		obj = new Entity(this);
		t = OBJ_ENTITY;
	}
	if( type == "Camera" )
	{ 
		obj = new Camera(this); 
		t = OBJ_CAMERA;
	}
	if( type == "Light" )
	{
		obj = new Light(this);
		t = OBJ_LIGHT;
	}

	if( obj )
	{
		_session->registerObject(obj, t, id);
		assert( obj->getID() != vl::ID_UNDEFINED );
		_objects.push_back(obj);
	}

	return obj;
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
	std::clog << "Printing bounding boxes." << std::endl;

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
vl::SceneManager::addToSelection( vl::SceneNodePtr node )
{
	if( !isInSelection(node) )
	{
		_selection.push_back(node);
		node->showBoundingBox(true);
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
		node->showBoundingBox(false);
	}
}

bool
vl::SceneManager::isInSelection( vl::SceneNodePtr node ) const
{
	SceneNodeList::const_iterator iter;
	for( iter = _selection.begin(); iter != _selection.end(); ++iter )
	{
		if( *iter == node )
		{ return true; }
	}

	return false;
}

/// -------------------------------Protected -----------------------------------
void
vl::SceneManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
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
	
	// @todo fix the serialization crashes, because of the std::string
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

	if(dirtyBits & DIRTY_SHADOW_INFO)
	{
		msg >> _shadows;

		/// ideally most of these should be configurable from config file
		/// because they are really performance intense, can look pretty crappy
		/// and most of them need to be static during the simulation

		_ogre_sm->setShadowTechnique( getOgreShadowTechnique(_shadows.technique) );
		_ogre_sm->setShadowColour(_shadows.colour);
		vl::to_lower(_shadows.camera);
		Ogre::ShadowCameraSetupPtr cam_setup;
		if( _shadows.camera == "default" )
		{
			cam_setup.bind(new Ogre::DefaultShadowCameraSetup());
		}
		else if( _shadows.camera == "planeoptimal" )
		{
			/// @todo needs a plane of interest
			// cam_setup.bind(new Ogre::PlaneOptimalShadowCameraSetup());
		}
		else if( _shadows.camera == "lispsm" )
		{
			/// LiSPSM has creately softer shadows compared to the default one
			/// i.e. less pixelisation in the edges.
			cam_setup.bind(new Ogre::LiSPSMShadowCameraSetup());
		}

		if( !cam_setup.isNull() )
		{ _ogre_sm->setShadowCameraSetup(cam_setup); }

		/// @todo for self shadowing we need to use custom shaders
		/// @todo make configurable
		_ogre_sm->setShadowTextureSelfShadow(true);
		/// Hard coded floating point texture, needs current hardware
		/// provides much better depth map
		_ogre_sm->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
		_ogre_sm->setShadowTextureCasterMaterial("ShadowCaster");

		/// For fixed functionality texture shadows to work this must be set
		/// @todo make configurable
		/// @todo add support for it in Shaders (just clip the shadow)
		_ogre_sm->setShadowFarDistance(50);
	}
}

/// ------------------------------- Private ----------------------------------
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

/// --------------------------------- Global ---------------------------------
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
	os << "Shadows : technique " << shadows.getShadowTechnique()
		<< " : colour " << shadows.colour
		<< " : camera " << shadows.camera;

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
	msg << shadows.technique << shadows.colour << shadows.camera;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::ShadowInfo &shadows)
{
	msg >> shadows.technique >> shadows.colour >> shadows.camera;

	return msg;
}
