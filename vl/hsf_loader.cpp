/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file hsf_loader.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

// Interface
#include "hsf_loader.hpp"

// Managers
#include "game_manager.hpp"
#include "scene_manager.hpp"
#include "mesh_manager.hpp"
#include "physics/physics_world.hpp"

#include "scene_node.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"

#include "base/string_utils.hpp"
#include "ogre_xml_helpers.hpp"

#include "game_object.hpp"

// Necessary for physics import
#include "physics/shapes.hpp"

// Necessary for disabling and enabling auto collision shape creation
#include "animation/kinematic_world.hpp"

vl::HSFLoader::HSFLoader(void)
{}

vl::HSFLoader::~HSFLoader(void)
{}

void
vl::HSFLoader::parseScene( std::string const &scene_data,
						   vl::GameManagerPtr game_manager,
						   std::string const &sPrependNode )
{
	_game = game_manager;
	assert(_game);

	_sPrependNode = sPrependNode;

	char *xml_data = new char[scene_data.length()+1];
	::strcpy( xml_data, scene_data.c_str() );

	_parse( xml_data );
}



void
vl::HSFLoader::parseScene( vl::TextResource &scene_data,
						   vl::GameManagerPtr game_manager,
						   std::string const &sPrependNode )
{
	_game = game_manager;
	assert(_game);

	// We need to disable auto creation of collision objects because they are
	// well defined in the HSF file itself.
	// But we want to maintain compatibility with older scripts and scene files 
	// which do not have these features so we restore it after we are done.
	assert(_game->getKinematicWorld());
	bool col_detection = _game->getKinematicWorld()->isCollisionDetectionEnabled();
	_game->getKinematicWorld()->enableCollisionDetection(false);

	_sPrependNode = sPrependNode;
	
	// Get the ownership of the memory
	char *xml_data = scene_data.get();

	if( !xml_data || ::strlen( xml_data ) != scene_data.size()-1 )
	{
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc("TextResource has invalid XML file") );
	}

	_parse( xml_data );

	_game->getKinematicWorld()->enableCollisionDetection(col_detection);

	// Reset data so that we don't end up with dangling pointers (or holding resources)
	_game = 0;
}

void
vl::HSFLoader::_parse(char *xml_data)
{
	rapidxml::xml_document<> XMLDoc;    // character type defaults to char

	rapidxml::xml_node<>* xml_root;

	XMLDoc.parse<0>( xml_data );

	// Grab the scene node
	xml_root = XMLDoc.first_node("scene");

	// @todo these can be removed after we can save hsf files because
	// they are only necessary for Ogre Scene files
	//
	// OgreMax exports angles in Radians by default so if the scene file is
	// created with Maya we assume Radians
	// Blender how ever uses Degrees by default so we will assume Degrees otherwise
	std::string app( vl::getAttrib(xml_root, "application", "") );
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
vl::HSFLoader::processScene(rapidxml::xml_node<> *xml_root)
{
	rapidxml::xml_node<>* pElement;

	// @todo add checking for multiple nodes
	// it should throw an error for invalid file.

	// Process environment (?)
	pElement = xml_root->first_node("environment");
	if(pElement)
	{ processEnvironment(pElement); }

	// Process nodes (?)
	pElement = xml_root->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }
}

void
vl::HSFLoader::processNodes(rapidxml::xml_node<> *xml_node)
{
	rapidxml::xml_node<>* pElement;

	// Process node (*)
	pElement = xml_node->first_node("node");
	while(pElement)
	{
		processNode(pElement);
		pElement = pElement->next_sibling("node");
	}
}

void
vl::HSFLoader::processEnvironment(rapidxml::xml_node<> *xml_node)
{
	std::clog << "vl::HSFLoader::processEnvironment" << std::endl;

	rapidxml::xml_node<> *pElement;

	// Process fog (?)
	pElement = xml_node->first_node("fog");
	if(pElement)
	{ processFog(pElement); }

	// Process skyDome (?)
	pElement = xml_node->first_node("sky");
	if( pElement )
	{ processSky(pElement); }

	// Process colourAmbient (?)
	pElement = xml_node->first_node("colourAmbient");
	if( pElement )
	{ _game->getSceneManager()->setAmbientLight( vl::parseColour(pElement) ); }
}

void
vl::HSFLoader::processFog(rapidxml::xml_node<> *xml_node)
{
	std::clog << "vl::HSFLoader::processFog" << std::endl;
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
	_game->getSceneManager()->setFog( vl::FogInfo(mode, colourDiffuse, expDensity, linearStart, linearEnd) );
}

void
vl::HSFLoader::processSky(rapidxml::xml_node<> *xml_node)
{
	std::clog << "vl::HSFLoader::processSky" << std::endl;
	// @todo select sky simulation and sky dome

	// Process attributes
	// material attribute is required, all others are optional and have defaults
	std::string material = xml_node->first_attribute("material")->value();
	Ogre::Real curvature = vl::getAttribReal(xml_node, "curvature", 10);
	Ogre::Real tiling = vl::getAttribReal(xml_node, "tiling", 8);
	Ogre::Real distance = vl::getAttribReal(xml_node, "distance", 4000);
	bool drawFirst = vl::getAttribBool(xml_node, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
	pElement = xml_node->first_node("rotation");
	if(pElement)
	{ rotation = vl::parseQuaternion(pElement); }

	// Setup the sky dome
	_game->getSceneManager()->setSkyDome( vl::SkyDomeInfo(material, curvature, tiling,
			distance, drawFirst, rotation, 16, 16, -1) );
}

void
vl::HSFLoader::processNode(rapidxml::xml_node<> *xml_node)
{
	// Construct the node's name
	std::string name = _sPrependNode + vl::getAttrib(xml_node, "name");

	// Create the scene node
	vl::GameObjectRefPtr node = _game->createGameObject(name);

	rapidxml::xml_node<>* pElement;

	bool dynamic = false;
	bool kinematic = false;
	std::string physics_engine_name = vl::getAttrib(xml_node, "physics_type");
	if(physics_engine_name == "auto")
	{
		std::cout << "Object : " << name 
			<< " requested \"auto\" physics engine which is not supported."
			<< " Falling back to kinematic." << std::endl;
		kinematic = true;
	}
	else if(physics_engine_name == "kinematic")
	{
		kinematic = true;
	}
	else if(physics_engine_name == "dynamic")
	{
		dynamic = true;
	}
	// defaults to none

	// parse collisions
	bool collision_detection = false;
	std::string collision_mesh_name;
	pElement = xml_node->first_node("collision");
	if(pElement)
	{
		// @todo add type parameter
		collision_detection = vl::getAttribBool(pElement, "enabled");
		collision_mesh_name = vl::getAttrib(pElement, "model");
	}

	/// Create RigidBody for the entity
	if(collision_detection)
	{
		// @todo add support for collision primitive

		// GameObject handles creation of collision model if one is not present here
		if(!collision_mesh_name.empty())
		{
			// Load the collision mesh
			vl::MeshRefPtr mesh = _game->getMeshManager()->getMesh(collision_mesh_name);
			assert(mesh);
			vl::physics::ConvexHullShapeRefPtr shape = vl::physics::ConvexHullShape::create(mesh);
			node->setCollisionModel(shape);
		}
	}

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
	// @todo add support, good question is what do we modify with the scale
	// because scaling causes always problems.
	pElement = xml_node->first_node("scale");
	if(pElement)
	{
		Ogre::Vector3 s = vl::parseVector3(pElement);
		if(!vl::equal(s, Ogre::Vector3(1, 1, 1)))
		{
			std::clog << "Scale parameter on node " << node->getName() 
				<< " scaling is not supported." << std::endl;
		}
	}

	/*	Process node (*)
	Needs to be here because the node can have children
	*/
	pElement = xml_node->first_node("node");
	while(pElement)
	{
		processChildNode(pElement, node->getGraphicsNode());
		pElement = pElement->next_sibling("node");
	}

	/*	Process entity (*) */
	pElement = xml_node->first_node("entity");
	while(pElement)
	{
		processEntity(pElement, node->getGraphicsNode());
		pElement = pElement->next_sibling("entity");
	}

	/*	Process light (*) */
	pElement = xml_node->first_node("light");
	while(pElement)
	{
		processLight(pElement, node->getGraphicsNode());
		pElement = pElement->next_sibling("light");
	}

	/*	Process camera (*) */
	pElement = xml_node->first_node("camera");
	while(pElement)
	{
		processCamera(pElement, node->getGraphicsNode());
		pElement = pElement->next_sibling("camera");
	}

	assert(!(dynamic && kinematic));

	// Entity needs to be processed before we can create dynamic or kinematic body
	// because they need the collision mesh if it wasn't provided separately.

	if(dynamic)
	{
		/// Process body node
		/// Because these have only effect for dynamics objects and we don't allow
		/// for switching types at run time just yet only process them with valid rigid body.
		Ogre::Vector3 inertia(1, 1, 1);
		Ogre::Real mass = 0;
		pElement = xml_node->first_node("body");
		if(pElement)
		{
			// Parse attributes
			// Default to static object
			Ogre::Real mass = vl::getAttribReal(pElement, "mass", 0);

			rapidxml::xml_node<> *inertia_xml = xml_node->first_node("inertia");
			
			if(inertia_xml)
			{ inertia = vl::parseVector3(inertia_xml); }
		}

		node->createRigidBody(mass, inertia);
	}

	if(kinematic)
	{
		node->setKinematic(true);
		if(collision_detection)
		{
			node->enableCollisionDetection(true);
		}
	}
}

void
vl::HSFLoader::processChildNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Construct the node's name
	std::string name = _sPrependNode + vl::getAttrib(xml_node, "name");

	// Create the scene node
	vl::SceneNodePtr node = parent->createChildSceneNode(name);

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
		processChildNode(pElement, node);
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
vl::HSFLoader::processEntity(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Process attributes
	std::string base_name = vl::getAttrib(xml_node, "name");
	std::string meshFile = vl::getAttrib(xml_node, "mesh_file");

	/// @todo should be removed after we have file saving as we don't need compatibility
	/// with Ogre scene file exporters after that.
	///
	/// Get an unique name for the entity
	/// This is mostly because of problematic Blender exporter that copies the
	/// entity name from the mesh name.
	uint16_t index = 0;
	std::stringstream name_ss(base_name);
	while( _game->getSceneManager()->hasEntity(name_ss.str()) )
	{
		name_ss.str("");
		name_ss << base_name << "_" << index;
		++index;
	}

	// Create the entity
	vl::EntityPtr entity = _game->getSceneManager()->createEntity(name_ss.str(), meshFile, true);
	//entity->setCastShadows(castShadows);
	parent->attachObject(entity);

//	if( !materialFile.empty() )
//	{ entity->setMaterialName(materialFile); }
}

void
vl::HSFLoader::processLight(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);
	assert(_game->getSceneManager());

	// Process attributes
	std::string name = vl::getAttrib(xml_node, "name");

	// Create the light
	vl::LightPtr light = _game->getSceneManager()->createLight(name);

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
	bool shadow = vl::getAttribBool(xml_node, "shadow", true);
	bool castShadows = vl::getAttribBool(xml_node, "castShadows", true);
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
vl::HSFLoader::processCamera(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Process attributes
	std::string name = vl::getAttrib(xml_node, "name");
	std::string id = vl::getAttrib(xml_node, "id");

	// Create the camera
	vl::CameraPtr camera = _game->getSceneManager()->createCamera( name );
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
vl::HSFLoader::processLightRange(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	// Process attributes
	Ogre::Real inner = vl::getAttribReal(xml_node, "inner");
	Ogre::Real outer = vl::getAttribReal(xml_node, "outer");
	Ogre::Real falloff = vl::getAttribReal(xml_node, "falloff", 1.0);

	// Setup the light range
	light->setSpotlightRange(Ogre::Angle(inner), Ogre::Angle(outer), falloff);
}

void
vl::HSFLoader::processLightAttenuation(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	// Process attributes
	Ogre::Real range = vl::getAttribReal(xml_node, "range");
	Ogre::Real constant = vl::getAttribReal(xml_node, "constant");
	Ogre::Real linear = vl::getAttribReal(xml_node, "linear");
	Ogre::Real quadratic = vl::getAttribReal(xml_node, "quadratic");

	// Setup the light attenuation
	light->setAttenuation( LightAttenuation(range, constant, linear, quadratic) );
}
