/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include "dotscene_loader.hpp"

#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"

#include "base/string_utils.hpp"
#include "ogre_xml_helpers.hpp"

vl::DotSceneLoader::DotSceneLoader()
{}

vl::DotSceneLoader::~DotSceneLoader()
{}

void
vl::DotSceneLoader::parseDotScene( const std::string& scene_data,
								   vl::SceneManager *scene,
								   vl::SceneNode* attachNode,
								   const std::string& sPrependNode )
{
	// set up shared object values
	_scene = scene;
	_sPrependNode = sPrependNode;

	char *xml_data = new char[scene_data.length()+1];
	::strcpy( xml_data, scene_data.c_str() );

	_parse( xml_data );
}



void
vl::DotSceneLoader::parseDotScene( vl::TextResource &scene_data,
								   vl::SceneManagerPtr scene,
								   vl::SceneNodePtr attachNode,
								   const std::string& sPrependNode )
{
	// set up shared object values
	_scene = scene;
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

	_parse( xml_data );
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
	std::string version = vl::getAttrib(xml_root, "formatVersion", "unknown");

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
	bool drawFirst = vl::getAttribBool(xml_node, "drawFirst", true);

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

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = xml_node->first_node("position");
	if( pElement )
	{ node->setPosition(vl::parseVector3(pElement)); }

	// Process rotation (?)
	pElement = xml_node->first_node("rotation");
	if( pElement )
	{ node->setOrientation(vl::parseQuaternion(pElement)); }

	pElement = xml_node->first_node("quaternion");
	if( pElement )
	{ node->setOrientation(vl::parseQuaternion(pElement)); }

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
	std::string name = vl::getAttrib(xml_node, "name");
	std::string id = vl::getAttrib(xml_node, "id");
	std::string meshFile = vl::getAttrib(xml_node, "meshFile");
	std::string materialFile = vl::getAttrib(xml_node, "materialFile");
	bool castShadows = vl::getAttribBool(xml_node, "castShadows", true);

	// Create the entity
	vl::EntityPtr entity = _scene->createEntity(name, meshFile);
	entity->setCastShadows(castShadows);
	parent->attachObject(entity);

	if( !materialFile.empty() )
	{ entity->setMaterialName(materialFile); }
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
	if(sValue == "point")
	{ light->setType( vl::Light::LT_POINT ); }
	else if(sValue == "directional")
	{ light->setType( vl::Light::LT_DIRECTIONAL ); }
	// The correct value from specification is spotLight
	// but OgreMax uses spot so we allow
	else if(sValue == "spot" || sValue == "spotLight" )
	{ light->setType( vl::Light::LT_SPOT ); }
	else if(sValue == "radPoint")
	{ light->setType( vl::Light::LT_POINT ); }

	light->setVisible(vl::getAttribBool(xml_node, "visible", true));
	light->setCastShadows(vl::getAttribBool(xml_node, "castShadows", true));

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
	if(pElement)
	{ light->setDiffuseColour(vl::parseColour(pElement)); }

	// Process colourSpecular (?)
	pElement = xml_node->first_node("colourSpecular");
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
