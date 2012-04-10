/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file dotscene_loader.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "dotscene_loader.hpp"

#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"

#include "base/string_utils.hpp"
#include "base/xml_helpers.hpp"

// Necessary for physics import
#include "physics/shapes.hpp"
#include "physics/physics_world.hpp"

vl::DotSceneLoader::DotSceneLoader(bool use_new_mesh_manager)
	: _use_new_mesh_manager(use_new_mesh_manager)
{}

vl::DotSceneLoader::~DotSceneLoader()
{}

void
vl::DotSceneLoader::parseDotScene( const std::string& scene_data,
								   vl::SceneManager *scene,
								   vl::physics::WorldRefPtr physics_world,
								   vl::SceneNode* attachNode,
								   const std::string& sPrependNode )
{
	// set up shared object values
	_scene = scene;
	_physics_world = physics_world;
	_sPrependNode = sPrependNode;

	char *xml_data = new char[scene_data.length()+1];
	::strcpy( xml_data, scene_data.c_str() );

	_parse( xml_data );
}



void
vl::DotSceneLoader::parseDotScene( vl::TextResource &scene_data,
								   vl::SceneManagerPtr scene,
								   vl::physics::WorldRefPtr physics_world,
								   vl::SceneNodePtr attachNode,
								   const std::string& sPrependNode )
{
	// set up shared object values
	_scene = scene;
	_physics_world = physics_world;
	_sPrependNode = sPrependNode;
	_attach_node = attachNode;
	if( !_attach_node )
	{ _attach_node = _scene->getRootSceneNode(); }
	
	// Pass the ownership of the memory to this
	char *xml_data = scene_data.get();

	if( !xml_data || ::strlen( xml_data ) != scene_data.size()-1 )
	{
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc("TextResource has invalid XML file") );
	}

	std::clog << "SceneNodes loaded from file = " << scene_data.getName() << std::endl;
	_file_name = scene_data.getName();
	_parse( xml_data );

	// Reset data so that we don't end up with dangling pointers (or holding resources)
	_physics_world.reset();
	_scene = 0;
}

void
vl::DotSceneLoader::_parse(char *xml_data)
{
	rapidxml::xml_document<> XMLDoc;    // character type defaults to char

	rapidxml::xml_node<>* xml_root;

	XMLDoc.parse<0>( xml_data );

	// Grab the scene node
	xml_root = XMLDoc.first_node("scene");

	// OgreMax exports angles in Radians by default so if the scene file is
	// created with Maya we assume Radians
	// Blender how ever uses Degrees by default so we will assume Degrees otherwise
	std::string app( vl::getAttrib(xml_root, "application") );
	vl::to_lower( app );
	// Mind you we might process multiple scene files some made with Maya and
	// some with Blender so this setting needs to be changed for each file.
	if( app == "maya" )
	{
		std::cout << "Processing Maya scene file." << std::endl;
		Ogre::Math::setAngleUnit( Ogre::Math::AU_RADIAN );
	}
	else
	{
		std::cout << "Processing Blender scene file." << std::endl;
		Ogre::Math::setAngleUnit( Ogre::Math::AU_DEGREE );
	}

	// Process the scene
	processScene(xml_root);
}


/// ------- DotSceneLoader Private -------------
void
vl::DotSceneLoader::processScene(rapidxml::xml_node<> *xml_root)
{
	parseSceneHeader(xml_root);

	rapidxml::xml_node<>* pElement;

	// Process environment (?)
	pElement = xml_root->first_node("environment");
	if(pElement)
	{ processEnvironment(pElement); }

	// Process nodes (?)
	pElement = xml_root->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }

	/// @todo why only one node is supported?
	// Process light (?)
	pElement = xml_root->first_node("light");
	if(pElement)
	{ processLight(pElement, _attach_node); }

	// Process camera (?)
	pElement = xml_root->first_node("camera");
	if(pElement)
	{ processCamera(pElement, _attach_node); }
}

void
vl::DotSceneLoader::parseSceneHeader(rapidxml::xml_node<> *xml_root)
{
	// Process the scene parameters
	std::string version = vl::getAttrib<std::string>(xml_root, "formatVersion", "unknown");

	std::string message = "[DotSceneLoader] Parsing dotScene file with version " + version;
	if(xml_root->first_attribute("ID"))
	{
		message += ", id " + std::string(xml_root->first_attribute("ID")->value());
	}
	if(xml_root->first_attribute("sceneManager"))
	{
		message += ", scene manager "
			+ std::string(xml_root->first_attribute("sceneManager")->value());
	}
	if(xml_root->first_attribute("minOgreVersion"))
	{
		message += ", min. Ogre version "
			+ std::string(xml_root->first_attribute("minOgreVersion")->value());
	}
	if(xml_root->first_attribute("author"))
	{
		message += ", author "
			+ std::string(xml_root->first_attribute("author")->value());
	}

	std::cout << message << std::endl;

}

void
vl::DotSceneLoader::processNodes(rapidxml::xml_node<> *xml_node)
{
	rapidxml::xml_node<>* pElement;

	// Process node (*)
	pElement = xml_node->first_node("node");
	while(pElement)
	{
		processNode(pElement, _attach_node);
		pElement = pElement->next_sibling("node");
	}

	// NOTE these are weird why do we want to reset the attach node?
	// Shouldn't we create another Node as the child of _attach_node
	// and modify that one instead?
	// Also why these are after processNode ?

	// Process position (?)
	pElement = xml_node->first_node("position");
	if(pElement)
	{ _attach_node->setPosition(vl::parseVector3(pElement)); }

	// Process rotation (?)
	pElement = xml_node->first_node("rotation");
	if(pElement)
	{ _attach_node->setOrientation(vl::parseQuaternion(pElement)); }
	
	pElement = xml_node->first_node("rotation");
	if( pElement )
	{ _attach_node->setOrientation(vl::parseQuaternion(pElement)); }

	// Process scale (?)
	pElement = xml_node->first_node("scale");
	if(pElement)
	{ _attach_node->setScale(vl::parseVector3(pElement)); }
}

void
vl::DotSceneLoader::processEnvironment(rapidxml::xml_node<> *xml_node)
{
	rapidxml::xml_node<> *pElement;

	// Process camera (?)
	pElement = xml_node->first_node("camera");
	if(pElement)
	{ processCamera(pElement, _attach_node); }

	// Process fog (?)
	pElement = xml_node->first_node("fog");
	if(pElement)
	{ processFog(pElement); }

	// Process skyDome (?)
	pElement = xml_node->first_node("skyDome");
	if( pElement )
	{ processSkyDome(pElement); }

	// Process colourAmbient (?)
	pElement = xml_node->first_node("colourAmbient");
	if( pElement )
	{ _scene->setAmbientLight( vl::parseColour(pElement) ); }
}

void
vl::DotSceneLoader::processFog(rapidxml::xml_node<> *xml_node)
{
	// Process attributes
	Ogre::Real expDensity = vl::getAttribReal(xml_node, "density", 0.001);
	Ogre::Real linearStart = vl::getAttribReal(xml_node, "start", 0.0);
	Ogre::Real linearEnd = vl::getAttribReal(xml_node, "end", 1.0);

	vl::FogMode mode = vl::FOG_NONE;
	std::string sMode = vl::getAttrib(xml_node, "mode");
	if(sMode == "none")
	{ mode = vl::FOG_NONE; }
	else if(sMode == "exp")
	{ mode = vl::FOG_EXP; }
	else if(sMode == "exp2")
	{ mode = vl::FOG_EXP2; }
	else if(sMode == "linear")
	{ mode = vl::FOG_LINEAR; }

	rapidxml::xml_node<> *pElement;

	// Process colourDiffuse (?)
	Ogre::ColourValue colourDiffuse = Ogre::ColourValue(1.0, 1.0, 1.0, 1.0);
	pElement = xml_node->first_node("colour");
	if(pElement)
	{ colourDiffuse = vl::parseColour(pElement); }

	// Setup the fog
	_scene->setFog( vl::FogInfo(mode, colourDiffuse, expDensity, linearStart, linearEnd) );
}

void
vl::DotSceneLoader::processSkyDome(rapidxml::xml_node<> *xml_node)
{
	// Process attributes
	// material attribute is required, all others are optional and have defaults
	std::string material = xml_node->first_attribute("material")->value();
	Ogre::Real curvature = vl::getAttribReal(xml_node, "curvature", 10);
	Ogre::Real tiling = vl::getAttribReal(xml_node, "tiling", 8);
	Ogre::Real distance = vl::getAttribReal(xml_node, "distance", 4000);
	bool drawFirst = vl::getAttrib(xml_node, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
	pElement = xml_node->first_node("rotation");
	if(pElement)
	{ rotation = vl::parseQuaternion(pElement); }

	// Setup the sky dome
	_scene->setSkyDome( vl::SkyDomeInfo(material, curvature, tiling,
			distance, drawFirst, rotation, 16, 16, -1) );
}

void
vl::DotSceneLoader::processNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	// Parents are not supported yet
	assert(parent);

	// Construct the node's name
	std::string name = _sPrependNode + vl::getAttrib(xml_node, "name");

	// Create the scene node
	vl::SceneNodePtr node = parent->createChildSceneNode(name);
	node->setSceneFile(_file_name);

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = xml_node->first_node("position");
	if( pElement )
	{ node->setPosition(vl::parseVector3(pElement)); }

	// Process rotation (?)
	pElement = xml_node->first_node("quaternion");
	if(!pElement)
	{ pElement = xml_node->first_node("rotation"); }

	if( pElement )
	{
		Ogre::Quaternion q = vl::parseQuaternion(pElement);
		node->setOrientation(q);
	}

	// Process scale (?)
	pElement = xml_node->first_node("scale");
	if(pElement)
	{ node->setScale(vl::parseVector3(pElement)); }

	/*	Process node (*)
	Needs to be here because the node can have children
	*/
	pElement = xml_node->first_node("node");
	while(pElement)
	{
		processNode(pElement, node);
		pElement = pElement->next_sibling("node");
	}

	/*	Process entity (*) */
	pElement = xml_node->first_node("entity");
	while(pElement)
	{
		processEntity(pElement, node);
		pElement = pElement->next_sibling("entity");
	}

	/*	Process light (*) */
	pElement = xml_node->first_node("light");
	while(pElement)
	{
		processLight(pElement, node);
		pElement = pElement->next_sibling("light");
	}

	/*	Process camera (*) */
	pElement = xml_node->first_node("camera");
	while(pElement)
	{
		processCamera(pElement, node);
		pElement = pElement->next_sibling("camera");
	}
}

void
vl::DotSceneLoader::processEntity(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Process attributes
	std::string base_name = vl::getAttrib(xml_node, "name");
	std::string id = vl::getAttrib(xml_node, "id");
	std::string meshFile = vl::getAttrib(xml_node, "meshFile");
	std::string materialFile = vl::getAttrib(xml_node, "materialFile");
	bool castShadows = vl::getAttrib(xml_node, "castShadows", true);

	/// Get an unique name for the entity
	/// This is mostly because of problematic Blender exporter that copies the
	/// entity name from the mesh name.
	uint16_t index = 0;
	std::stringstream name_ss(base_name);
	while( _scene->hasEntity(name_ss.str()) )
	{
		name_ss.str("");
		name_ss << base_name << "_" << index;
		++index;
	}

	// Create the entity
	vl::EntityPtr entity = _scene->createEntity(name_ss.str(), meshFile, _use_new_mesh_manager);
	entity->setCastShadows(castShadows);
	parent->attachObject(entity);

	if( !materialFile.empty() )
	{ entity->setMaterialName(materialFile); }

	/// Create RigidBody for the entity
	if( _physics_world )
	{
		// @todo would it just be easier to enable the mesh manager automatically?
		if(!_use_new_mesh_manager)
		{
			std::string msg("Physics needs the new mesh manager.");
			BOOST_THROW_EXCEPTION(vl::invalid_dotscene() << vl::desc(msg));
		}

		// Not checking collision primitive, only tiangle_mesh is supported
		bool actor = vl::getAttrib(xml_node, "actor", false);
		Ogre::Real damping_rot = vl::getAttribReal(xml_node, "damping_rot", 0.1);
		Ogre::Real damping_trans = vl::getAttribReal(xml_node, "damping_trans", 0.1);
		// Friction and ghost not supported
		// Inertia is only a scalar
		Ogre::Real inertia = vl::getAttribReal(xml_node, "inertia_tensor", 1.0);
		// lock_rot and lock_trans not supported
		Ogre::Real mass = vl::getAttribReal(xml_node, "mass", 1);
		Ogre::Real mass_radius = vl::getAttribReal(xml_node, "mass_radius", 1.0);
		// defaults to rigid body, should probably be static
		std::string type = vl::getAttrib<std::string>(xml_node, "physics_type", "RIGID_BODY");
		// velociy_max and velocity_min not supported

		vl::Transform transform(parent->getWorldTransform());
		vl::physics::MotionState *m_state = _physics_world->createMotionState(transform, parent);
		vl::MeshRefPtr mesh = _scene->getMeshManager()->getMesh(meshFile);
		assert(mesh);
		// @todo static mesh support for static objects
		// @todo add support for NO_COLLISION objects
		vl::physics::ConvexHullShapeRefPtr shape = vl::physics::ConvexHullShape::create(mesh);
		// debug set mass to 1
		if(mass == 0)
		{ inertia = 0; }
		
		if(type == "RIGID_BODY")
		{
			std::clog << "Creating body " << name_ss.str() << " with mass " << mass 
				<< " damping : linear " << damping_trans << " rotational " << damping_rot 
				<< std::endl;
			physics::RigidBodyRefPtr body = _physics_world->createRigidBody(name_ss.str(), mass, m_state, shape, Ogre::Vector3(1,1,1));
			body->setUserControlled(actor);
			if(actor)
			{ std::clog << "Actor enabled." << std::endl; }
			body->setDamping(damping_trans, damping_rot);
		}
		else if(type == "NO_COLLISION")
		{
			std::clog << "Entity with no collision. Will not create a RigidBody." << std::endl;
		}

		// @todo add support for compound collision objects, 
		// when they are in the SceneNode hierarchy
	}
}

void
vl::DotSceneLoader::processLight(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	// If no parent is provided we use Root node
	if( !parent )
	{ parent = _attach_node; }

	// Process attributes
	std::string name = vl::getAttrib(xml_node, "name");
	std::string id = vl::getAttrib(xml_node, "id");

	// Create the light
	vl::LightPtr light = _scene->createLight(name);

	parent->attachObject(light);

	std::string sValue = vl::getAttrib(xml_node, "type");
	if(sValue == "point" || sValue == "hemi" || sValue == "radPoint")
	{ light->setType( vl::Light::LT_POINT ); }
	else if(sValue == "directional")
	{ light->setType( vl::Light::LT_DIRECTIONAL ); }
	// The correct value from specification is spotLight
	// but OgreMax uses spot so we allow
	else if(sValue == "spot" || sValue == "spotLight" )
	{ light->setType( vl::Light::LT_SPOT ); }

	light->setVisible(vl::getAttribBool(xml_node, "visible", true));
	bool shadow = vl::getAttrib(xml_node, "shadow", true);
	bool castShadows = vl::getAttrib(xml_node, "castShadows", true);
	light->setCastShadows(shadow || castShadows);

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = xml_node->first_node("position");
	if(pElement)
	{ light->setPosition(vl::parseVector3(pElement)); }

	// Process normal (?)
	pElement = xml_node->first_node("normal");
	if(pElement)
	{ light->setDirection(vl::parseVector3(pElement)); }

	pElement = xml_node->first_node("directionVector");
	if(pElement)
	{ light->setDirection(vl::parseVector3(pElement)); }

	// Process colourDiffuse (?)
	pElement = xml_node->first_node("colourDiffuse");
	if(!pElement)
	{ pElement = xml_node->first_node("colorDiffuse"); }

	if(pElement)
	{ light->setDiffuseColour(vl::parseColour(pElement)); }

	// Process colourSpecular (?)
	pElement = xml_node->first_node("colourSpecular");
	if(!pElement)
	{ pElement = xml_node->first_node("colorSpecular"); }

	if(pElement)
	{ light->setSpecularColour(vl::parseColour(pElement)); }

	// Set the parameters wether or not the Light type supports them
	// They are filtered by the light anyway
	// and are usable if the light type is changed

	// Process lightRange (?)
	pElement = xml_node->first_node("lightRange");
	if(pElement)
	{ processLightRange(pElement, light); }

	pElement = xml_node->first_node("lightAttenuation");
	if(pElement)
	{ processLightAttenuation(pElement, light); }
}

void
vl::DotSceneLoader::processCamera(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Process attributes
	std::string name = vl::getAttrib(xml_node, "name");
	std::string id = vl::getAttrib(xml_node, "id");

	// Create the camera
	vl::CameraPtr camera = _scene->createCamera( name );
	parent->attachObject( camera );

	rapidxml::xml_node<> *pElement;

	// Process clipping (?)
	pElement = xml_node->first_node("clipping");
	if(pElement)
	{
		// Support both standard attribute name nearPlaneDist and
		// non-standard near used by OgreMax
		Ogre::Real nearDist = vl::getAttribReal(pElement, "near");
		if( nearDist == 0 )
		{ nearDist = vl::getAttribReal(pElement, "nearPlaneDist"); }

		if( nearDist > 0 )
		{ camera->setNearClipDistance(nearDist); }

		// Support both standard and non-standard attribute names
		Ogre::Real farDist =  vl::getAttribReal(pElement, "far");
		if( farDist == 0 )
		{ farDist = vl::getAttribReal(pElement, "farPlaneDist"); }

		if( farDist > 0 && farDist > nearDist )
		{ camera->setFarClipDistance(farDist); }
	}

	// Process position (?)
	pElement = xml_node->first_node("position");
	if(pElement)
	{ camera->setPosition( vl::parseVector3(pElement) ); }

	// Process rotation (?)
	pElement = xml_node->first_node("rotation");
	if(pElement)
	{ camera->setOrientation( vl::parseQuaternion(pElement) ); }

	pElement = xml_node->first_node("quaternion");
	if(pElement)
	{ camera->setOrientation( vl::parseQuaternion(pElement) ); }
}

void
vl::DotSceneLoader::processLightRange(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	// Process attributes
	Ogre::Real inner = vl::getAttribReal(xml_node, "inner");
	Ogre::Real outer = vl::getAttribReal(xml_node, "outer");
	Ogre::Real falloff = vl::getAttribReal(xml_node, "falloff", 1.0);

	// Setup the light range
	light->setSpotlightRange(Ogre::Angle(inner), Ogre::Angle(outer), falloff);
}

void
vl::DotSceneLoader::processLightAttenuation(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	// Process attributes
	Ogre::Real range = vl::getAttribReal(xml_node, "range");
	Ogre::Real constant = vl::getAttribReal(xml_node, "constant");
	Ogre::Real linear = vl::getAttribReal(xml_node, "linear");
	Ogre::Real quadratic = vl::getAttribReal(xml_node, "quadratic");

	// Setup the light attenuation
	light->setAttenuation( LightAttenuation(range, constant, linear, quadratic) );
}
