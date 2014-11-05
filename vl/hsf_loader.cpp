/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file hsf_loader.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
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
#include "base/xml_helpers.hpp"

#include "game_object.hpp"

// Necessary for physics import
#include "physics/shapes.hpp"
#include "physics/physics_constraints.hpp"

// Necessary for disabling and enabling auto collision shape creation
#include "animation/kinematic_world.hpp"
#include "animation/constraints.hpp"

vl::HSFLoader::HSFLoader(void)
{}

vl::HSFLoader::~HSFLoader(void)
{}

void
vl::HSFLoader::parseScene( std::string const &scene_data,
						   vl::GameManagerPtr game_manager, LOADER_FLAGS flags )
{
	_game = game_manager;
	_flags = flags;
	assert(_game);

	char *xml_data = new char[scene_data.length()+1];
	::strcpy( xml_data, scene_data.c_str() );

	_parse( xml_data );

	// Reset data so that we don't end up with dangling pointers (or holding resources)
	_game = 0;
}



void
vl::HSFLoader::parseScene( vl::TextResource &scene_data,
						   vl::GameManagerPtr game_manager, LOADER_FLAGS flags )
{
	_game = game_manager;
	_flags = flags;
	
	// Get the ownership of the memory
	char *xml_data = scene_data.get();

	if( !xml_data || ::strlen( xml_data ) != scene_data.size()-1 )
	{
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc("TextResource has invalid XML file") );
	}

	_parse( xml_data );

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
vl::HSFLoader::processScene(rapidxml::xml_node<> *xml_root)
{
	rapidxml::xml_node<>* pElement;

	assert(_game);

	// We need to disable auto creation of collision objects because they are
	// well defined in the HSF file itself.
	// But we want to maintain compatibility with older scripts and scene files 
	// which do not have these features so we restore it after we are done.
	assert(_game->getKinematicWorld());
	bool col_detection = _game->getKinematicWorld()->isCollisionDetectionEnabled();
	_game->getKinematicWorld()->enableCollisionDetection(false);

	// @todo add checking for multiple nodes
	// it should throw an error for invalid file.

	// Process environment (?)
	pElement = xml_root->first_node("environment");
	if(pElement)
	{ processEnvironment(pElement); }

	// HACK
	// Reset constraints
	// This needs to be done before reading the nodes so we preserve
	// the correct positions.
	// Proper fix would be to find the constraints for the specific node
	// and reset those instead of them all.
	vl::ConstraintList constraints = _game->getKinematicWorld()->getConstraints();
	for(vl::ConstraintList::iterator iter = constraints.begin(); 
		iter != constraints.end(); ++iter)
	{
		(*iter)->_getLink()->reset();
	}

	// Process nodes (?)
	pElement = xml_root->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }

	pElement = xml_root->first_node("constraints");
	if( pElement )
	{ processConstraints(pElement); }

	_game->getKinematicWorld()->enableCollisionDetection(col_detection);
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

	pElement = xml_node->first_node("shadows");
	if( pElement )
	{ processShadows(pElement); }

	// Process colourAmbient (?)
	pElement = xml_node->first_node("ambient_light");
	// Backward compatibility
	if(!pElement)
	{ pElement = xml_node->first_node("colourAmbient"); }
	if( pElement )
	{ _game->getSceneManager()->setAmbientLight( vl::parseColour(pElement) ); }
}

void
vl::HSFLoader::processFog(rapidxml::xml_node<> *xml_node)
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
	_game->getSceneManager()->setFog( vl::FogInfo(mode, colourDiffuse, expDensity, linearStart, linearEnd) );
}

void
vl::HSFLoader::processSky(rapidxml::xml_node<> *xml_node)
{
	/// Sky simulation
	rapidxml::xml_node<> *dynamic = xml_node->first_node("dynamic");
	if(dynamic)
	{
		std::string preset = vl::getAttrib<std::string>(dynamic, "preset", "none");
		if(preset != "none")
		{
			SkyInfo sky(preset);
			_game->getSceneManager()->setSkyInfo(sky);
		}
	}

	// Sky dome
	rapidxml::xml_node<> *dome = xml_node->first_node("static");
	if(dome)
	{
		vl::SkyDomeInfo info;
		// Process attributes
		info.material_name = vl::getAttrib(dome, "material", info.material_name);
		info.curvature = vl::getAttrib<vl::scalar>(dome, "curvature", info.curvature);
		info.tiling = vl::getAttrib<vl::scalar>(dome, "tiling", info.tiling);
		info.distance = vl::getAttrib<vl::scalar>(dome, "distance", info.distance);
		info.draw_first = vl::getAttrib(dome, "drawFirst", info.draw_first);

		// Process rotation (?)
		rapidxml::xml_node<> *pElement = dome->first_node("rotation");
		if(pElement)
		{ info.orientation = vl::parseQuaternion(pElement); }

		// Setup the sky dome
		_game->getSceneManager()->setSkyDome(info);
	}
}

void
vl::HSFLoader::processShadows(rapidxml::xml_node<> *xml_node)
{
	bool enabled = vl::getAttrib(xml_node, "enabled", false);
	std::string camera = vl::getAttrib(xml_node, "camera", std::string());
	bool shelf_shadows = vl::getAttrib(xml_node, "shelf_shadowing", false);
	int tex_size = vl::getAttrib(xml_node, "texture_size", 1024);
	vl::scalar max_distance = vl::getAttribReal(xml_node, "max_disantace", vl::scalar(250.0));
	std::string caster_material = vl::getAttrib(xml_node, "caster_material", std::string());
	
	// @todo add directional light offset
	ShadowInfo info(camera);
	info.setEnabled(enabled);
	info.setShelfShadowEnabled(shelf_shadows);
	info.setShadowCasterMaterial(caster_material);
	info.setTextureSize(tex_size);
	info.setMaxDistance(max_distance);
	_game->getSceneManager()->setShadowInfo(info);
}

void
vl::HSFLoader::processNode(rapidxml::xml_node<> *xml_node)
{
	// Construct the node's name
	std::string name = vl::getAttrib(xml_node, "name");

	if(name.empty())
	{
		// @todo replace with a real exception
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Invalid node name."));
	}

	// Create the game object
	vl::GameObjectRefPtr node;
	if( (_flags & LOADER_FLAG_OVERWRITE) && _game->hasGameObject(name) )
	{
		node = _game->getGameObject(name);
	}
	else if( (_flags & LOADER_FLAG_RENAME) && _game->hasGameObject(name) )
	{
		_game->hasGameObject(name);
		
		// Find a name that is available
		size_t counter = 0;
		std::stringstream new_name;
		new_name << name << "_" << counter;
		while(_game->hasGameObject(new_name.str()))
		{
			new_name.str("");
			++counter;
			new_name << name << "_" << counter;
		}

		node = _game->createGameObject(new_name.str());
	}
	// Default behavior we don't allow to read objects with duplicate names
	else
	{
		if(_game->hasGameObject(name))
		{ BOOST_THROW_EXCEPTION(vl::duplicate()); }

		node = _game->createGameObject(name);
	}

	assert(node);

	rapidxml::xml_node<>* pElement;

	bool dynamic = false;
	bool kinematic = false;
	std::string physics_engine_name = vl::getAttrib(xml_node, "physics_type", std::string());
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
		collision_detection = vl::getAttrib(pElement, "enabled", false);
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
			vl::MeshRefPtr mesh = _game->getMeshManager()->loadMesh(collision_mesh_name);
			vl::physics::ConvexHullShapeRefPtr shape = vl::physics::ConvexHullShape::create(mesh);
			node->setCollisionModel(shape);
		}
	}

	pElement = xml_node->first_node("transform");
	if(pElement)
	{
		Transform t;
		rapidxml::xml_node<> *pos = pElement->first_node("position");
		if(pos)
		{ t.position = vl::parseVector3(pos); }

		rapidxml::xml_node<> *orient = pElement->first_node("quaternion");
		if(orient)
		{ t.quaternion = vl::parseQuaternion(orient); }

		node->setTransform(t);
	}
	else
	{
		/// For backward compatibility
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

		// Create missing rigid bodies
		if(!node->getPhysicsNode())
		{ node->createRigidBody(mass, inertia); }
		// Otherwise reset parameters
		else
		{ node->getPhysicsNode()->setMassProps(mass, inertia); }
	}

	if(kinematic)
	{
		node->setKinematic(true);
		if(collision_detection)
		{
			node->enableCollisionDetection(true);
		}
	}

	assert(collision_detection == node->isCollisionDetectionEnabled());
}

void
vl::HSFLoader::processChildNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	std::string name = vl::getAttrib(xml_node, "name");
	
	if(name.empty())
	{ BOOST_THROW_EXCEPTION(vl::invalid_dotscene() << vl::desc("Node without name is not supported.")); }

	vl::SceneManagerPtr scene = _game->getSceneManager();
	// Create the scene node
	vl::SceneNodePtr node = 0;
	if( (_flags & LOADER_FLAG_OVERWRITE) && scene->hasSceneNode(name) )
	{
		node = scene->getSceneNode(name);
		assert(node);
		parent->addChild(node);
	}
	else if( (_flags & LOADER_FLAG_RENAME) && scene->hasSceneNode(name) )
	{	
		// Find a name that is available
		size_t counter = 0;
		std::stringstream new_name;
		new_name << name << "_" << counter;
		while(_game->getSceneManager()->hasSceneNode(new_name.str()))
		{
			new_name.str("");
			++counter;
			new_name << name << "_" << counter;
		}

		node = parent->createChildSceneNode(new_name.str());
	}
	// Default behavior we don't allow to read objects with duplicate names
	else
	{
		if( _game->getSceneManager()->hasSceneNode(name) )
		{ BOOST_THROW_EXCEPTION(vl::duplicate()); }

		node = parent->createChildSceneNode(name);
	}

	assert(node);

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
vl::HSFLoader::processBody(rapidxml::xml_node<> *xml_node, vl::GameObjectRefPtr obj)
{
	std::clog << "vl::HSFLoader::processBody" << std::endl;
}

void
vl::HSFLoader::processConstraints(rapidxml::xml_node<> *xml_node)
{
	std::clog << "vl::HSFLoader::processConstraints" << std::endl;
	rapidxml::xml_node<> *pElement = xml_node->first_node("constraint");
	while(pElement)
	{
		processConstraint(pElement);
		pElement = pElement->next_sibling("constraint");
	}	
}
	
void
vl::HSFLoader::processConstraint(rapidxml::xml_node<> *xml_node)
{
	std::clog << "vl::HSFLoader::processConstraint" << std::endl;

	std::string engine = vl::getAttrib(xml_node, "physics_type");
	std::string name = vl::getAttrib(xml_node, "name");
	std::string type = vl::getAttrib(xml_node, "type");
	std::string body_a = vl::getAttrib(xml_node, "body_a");
	std::string body_b = vl::getAttrib(xml_node, "body_b");
	bool actuator = vl::getAttrib(xml_node, "actuator", false);

	assert(!engine.empty() && !type.empty() && !body_a.empty() && !body_b.empty());

	rapidxml::xml_node<> *frame_a = xml_node->first_node("frame_a");
	rapidxml::xml_node<> *frame_b = xml_node->first_node("frame_b");

	assert(frame_a && frame_b);
	Transform fA = parseTransform(frame_a);
	Transform fB = parseTransform(frame_b);

	// @todo this can not be done this way because
	// sixdof constraint has more complex min and max
	rapidxml::xml_node<> *limit = xml_node->first_node("limit");
	std::string unit = vl::getAttrib(limit, "unit");
	vl::scalar min = vl::getAttrib<vl::scalar>(limit, "min", 0);
	vl::scalar max = vl::getAttrib<vl::scalar>(limit, "max", -1);

	Ogre::Vector3 axis(Ogre::Vector3::UNIT_Z);
	// Constraint does not have to define axis
	rapidxml::xml_node<> *xml_axis = xml_node->first_node("axis");
	if(xml_axis)
	{ axis = vl::parseVector3(xml_axis); }

	if(engine == "kinematic")
	{
		KinematicBodyRefPtr bodyA = _game->getKinematicWorld()->getKinematicBody(body_a);
		KinematicBodyRefPtr bodyB = _game->getKinematicWorld()->getKinematicBody(body_b);

		assert(bodyA && bodyB);

		if(name.empty())
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Constraint with no name.")); }

		vl::KinematicWorldRefPtr world = _game->getKinematicWorld();
		// Create the constraint
		ConstraintRefPtr con;
		if( (_flags & LOADER_FLAG_OVERWRITE) && world->getConstraint(name) )
		{
			con = world->getConstraint(name);
			// @todo this does not respect the bodyA and bodyB
			// they need to be reconfigured to the constraint or it needs to be remade.
			// remove old constraints because they can't be reconfigured
			// Not a good idea, all the pointers (from python) are lost
			// also we need to handle the removal properly
			//_game->getKinematicWorld()->removeConstraint(con);
			assert(con->getTypeName() == type);
			con->reset(bodyA, bodyB, fA, fB);
		}
		else if( (_flags & LOADER_FLAG_RENAME) && world->getConstraint(name) )
		{	
			// Find a name that is available
			size_t counter = 0;
			std::stringstream new_name;
			new_name << name << "_" << counter;
			while(world->getConstraint(new_name.str()))
			{
				new_name.str("");
				++counter;
				new_name << name << "_" << counter;
			}

			con = world->createConstraint(type, bodyA, bodyB, fA, fB, name);
		}
		// Default behavior we don't allow to read objects with duplicate names
		else
		{
			if( world->getConstraint(name) )
			{ BOOST_THROW_EXCEPTION(vl::duplicate()); }

			// throws if such type is not available
			con = world->createConstraint(type, bodyA, bodyB, fA, fB, name);
		}

		assert(con);

		// Set parameters
		con->setActuator(actuator);
		if(HingeConstraintRefPtr hinge = boost::dynamic_pointer_cast<HingeConstraint>(con))
		{
			Ogre::Radian min_, max_;
			if(unit == "degree")
			{
				min_ = Ogre::Radian(Ogre::Degree(min));
				max_ = Ogre::Radian(Ogre::Degree(max));
			}
			else
			{
				min_ = Ogre::Radian(min);
				max_ = Ogre::Radian(max);
			}

			hinge->setLowerLimit(min_);
			hinge->setUpperLimit(max_);
			hinge->setAxis(axis);
		}
		else if(SliderConstraintRefPtr slider = boost::dynamic_pointer_cast<SliderConstraint>(con))
		{
			slider->setLowerLimit(min);
			slider->setUpperLimit(max);
			slider->setAxis(axis);
		}
		// other constraints don't have limits
	}
	else if(engine == "dynamic")
	{
		/// @todo this is still a stub
		assert(_game->getPhysicsWorld());
		physics::RigidBodyRefPtr bodyA = _game->getPhysicsWorld()->getRigidBody(body_a);
		physics::RigidBodyRefPtr bodyB = _game->getPhysicsWorld()->getRigidBody(body_b);

		assert(bodyA && bodyB);

		vl::physics::ConstraintRefPtr constraint;

		// If we have a named constraint modify it's parameters instead of creating a new
		// @todo this functions like the OVERWRITE flag always, implement RENAME and DEFAULT also
		if(_game->getPhysicsWorld()->hasConstraint(name))
		{
			// Remove old constraints because they can't be reconfigured
			constraint = _game->getPhysicsWorld()->getConstraint(name);
			//_game->getPhysicsWorld()->removeConstraint(constraint);
			//constraint.reset();
			assert(type == constraint->getTypeName());
			constraint->reset(bodyA, bodyB, fA, fB);
		}
		else
		{
			// Create constraints
			// @todo parameter processing
			if(type == "hinge")
			{
				vl::physics::HingeConstraintRefPtr hinge = physics::HingeConstraint::create(bodyA, bodyB, fA, fB);
				constraint = hinge;
			}
			else if(type == "fixed")
			{
				assert(false && "Physics solver does not yet support fixed constraints.");
			}
			else if(type == "slider")
			{
				vl::physics::SliderConstraintRefPtr slider = physics::SliderConstraint::create(bodyA, bodyB, fA, fB);
				constraint = slider;
			}
			else if(type == "6dof")
			{
				vl::physics::SixDofConstraintRefPtr dof = physics::SixDofConstraint::create(bodyA, bodyB, fA, fB);
				constraint = dof;
			}
			else
			{
				assert(false && "Unsupported constraint type");
			}

			assert(constraint);
			_game->getPhysicsWorld()->addConstraint(constraint);
		}
	}
	else
	{
		assert(false && "Incorrect physics engine parameter");
	}
}

void
vl::HSFLoader::processEntity(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Process attributes
	std::string base_name = vl::getAttrib(xml_node, "name");
	std::string meshFile = vl::getAttrib(xml_node, "mesh_file");

	vl::SceneManagerPtr scene = _game->getSceneManager();

	// Create the entity
	vl::EntityPtr entity = 0;
	if( (_flags & LOADER_FLAG_OVERWRITE) && scene->hasEntity(base_name) )
	{
		entity = scene->getEntity(base_name);
	}
	/// Old default behavior here for compatibility
	else if(scene->hasEntity(base_name))
	{
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
		entity = _game->getSceneManager()->createEntity(name_ss.str(), meshFile, true);
	}
	else
	{ entity = _game->getSceneManager()->createEntity(base_name, meshFile, true); }
	
	assert(entity);

	//entity->setCastShadows(castShadows);
	parent->attachObject(entity);

	// @todo why there is no support for material?
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

	vl::SceneManagerPtr scene = _game->getSceneManager();
	
	// Create the light
	vl::LightPtr light = 0;
	if( _flags & LOADER_FLAG_OVERWRITE && scene->hasLight(name) )
	{
		light = scene->getLight(name);
	}
	else if(_flags & LOADER_FLAG_RENAME && scene->hasLight(name) )
	{
		uint16_t index = 0;
		std::stringstream name_ss(name);
		while( scene->hasLight(name_ss.str()) )
		{
			name_ss.str("");
			name_ss << name << "_" << index;
			++index;
		}
		light = scene->createLight(name_ss.str());
	}
	else
	{
		light = scene->createLight(name);
	}

	assert(light);

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

	light->setVisible(vl::getAttrib(xml_node, "visible", true));
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
vl::HSFLoader::processCamera(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent)
{
	assert(parent);

	// Process attributes
	std::string name = vl::getAttrib(xml_node, "name");
	std::string id = vl::getAttrib(xml_node, "id");

	vl::SceneManagerPtr scene = _game->getSceneManager();

	// Create the camera
	vl::CameraPtr camera;
	if( _flags & LOADER_FLAG_OVERWRITE && scene->hasCamera(name) )
	{
		camera = scene->getCamera(name);
	}
	else if(_flags & LOADER_FLAG_RENAME && scene->hasCamera(name))
	{
		uint16_t index = 0;
		std::stringstream name_ss(name);
		while( scene->hasCamera(name_ss.str()) )
		{
			name_ss.str("");
			name_ss << name << "_" << index;
			++index;
		}
		camera = scene->createCamera(name_ss.str());
	}
	else
	{
		camera = scene->createCamera(name);
	}

	assert(camera);
	
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
