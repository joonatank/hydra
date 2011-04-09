#include "ogre_dotscene_loader.hpp"

#include <sstream>

#include "base/string_utils.hpp"
#include "base/exceptions.hpp"
#include "ogre_xml_helpers.hpp"

// Necessary for logggin
#include <OGRE/OgreLogManager.h>

#include <OGRE/OgreException.h>
#include <OGRE/OgreResourceManager.h>
#include <OGRE/OgreEntity.h>

vl::ogre::DotSceneLoader::DotSceneLoader( void )
{}

vl::ogre::DotSceneLoader::~DotSceneLoader( void )
{}

void
vl::ogre::DotSceneLoader::parseDotScene( std::string const &scene_name,
									   Ogre::SceneManager* sceneMgr,
									   Ogre::SceneNode* attachNode,
									   const std::string& sPrependNode )
{
	// set up shared object values
	_scene_mgr = sceneMgr;
	_attach_node = attachNode;
	_sPrependNode = sPrependNode;

	// TODO replace asserts with throws
	Ogre::ResourceGroupManager &man = Ogre::ResourceGroupManager::getSingleton();
	assert( man.resourceExistsInAnyGroup(scene_name) );
	char *xml_data = ::strdup( man.openResource(scene_name).get()->getAsString().c_str() );
	assert( xml_data );

	_parse( xml_data );
}

void
vl::ogre::DotSceneLoader::_parse(char* xml_data)
{
	rapidxml::xml_document<> XMLDoc;    // character type defaults to char

	rapidxml::xml_node<>* XMLRoot;

	XMLDoc.parse<0>( xml_data );

	// Grab the scene node
	XMLRoot = XMLDoc.first_node("scene");

	// OgreMax exports angles in Radians by default so if the scene file is
	// created with Maya we assume Radians
	// Blender how ever uses Degrees by default so we will assume Degrees otherwise
	std::string app( vl::getAttrib(XMLRoot, "application", "") );
	vl::to_lower( app );
	// Mind you we might process multiple scene files some made with Maya and
	// some with Blender so this setting needs to be changed for each file.
	if( app == "maya" )
	{
		std::string message = "Processing Maya scene file.";
		Ogre::LogManager::getSingleton().logMessage(message);
		Ogre::Math::setAngleUnit( Ogre::Math::AU_RADIAN );
	}
	else
	{
		std::string message("Processing Blender scene file.");
		Ogre::LogManager::getSingleton().logMessage(message);
		Ogre::Math::setAngleUnit( Ogre::Math::AU_DEGREE );
	}

	if( !_attach_node )
	{ _attach_node = _scene_mgr->getRootSceneNode(); }

	// Process the scene
	processScene(XMLRoot);
}

void
vl::ogre::DotSceneLoader::processScene(rapidxml::xml_node<>* XMLRoot)
{
	rapidxml::xml_node<>* pElement;

	// Process environment (?)
	pElement = XMLRoot->first_node("environment");
	if(pElement)
	{ processEnvironment(pElement); }
}

void
vl::ogre::DotSceneLoader::processEnvironment(rapidxml::xml_node<>* XMLNode)
{
	rapidxml::xml_node<>* pElement;

	// Process fog (?)
	pElement = XMLNode->first_node("fog");
	if(pElement)
	{ processFog(pElement); }

	// Process skyBox (?)
	pElement = XMLNode->first_node("skyBox");
	if(pElement)
	{ processSkyBox(pElement); }

	// Process skyDome (?)
	pElement = XMLNode->first_node("skyDome");
	if( pElement )
	{ processSkyDome(pElement); }

	// Process skyPlane (?)
	pElement = XMLNode->first_node("skyPlane");
	if( pElement )
	{ processSkyPlane(pElement); }

	// Process colourAmbient (?)
	pElement = XMLNode->first_node("colourAmbient");
	if( pElement )
	{ _scene_mgr->setAmbientLight( vl::parseColour(pElement) ); }
}

void
vl::ogre::DotSceneLoader::processFog(rapidxml::xml_node<>* XMLNode)
{
	// Process attributes
	Ogre::Real expDensity = vl::getAttribReal(XMLNode, "density", 0.001);
	Ogre::Real linearStart = vl::getAttribReal(XMLNode, "start", 0.0);
	Ogre::Real linearEnd = vl::getAttribReal(XMLNode, "end", 1.0);

	Ogre::FogMode mode = Ogre::FOG_NONE;
	std::string sMode = vl::getAttrib(XMLNode, "mode");
	if(sMode == "none")
	{ mode = Ogre::FOG_NONE; }
	else if(sMode == "exp")
	{ mode = Ogre::FOG_EXP; }
	else if(sMode == "exp2")
	{ mode = Ogre::FOG_EXP2; }
	else if(sMode == "linear")
	{ mode = Ogre::FOG_LINEAR; }

	rapidxml::xml_node<>* pElement;

	// Process colourDiffuse (?)
	Ogre::ColourValue colourDiffuse = Ogre::ColourValue(1.0, 1.0, 1.0, 1.0);
	pElement = XMLNode->first_node("colour");
	if(pElement)
	{ colourDiffuse = vl::parseColour(pElement); }

	// Setup the fog
	_scene_mgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
}

void
vl::ogre::DotSceneLoader::processSkyBox(rapidxml::xml_node<>* XMLNode)
{
	// Process attributes
	// material attribute is required, all others are optional and have defaults
	std::string material = XMLNode->first_attribute("material")->value();
	Ogre::Real distance = vl::getAttribReal(XMLNode, "distance", 5000);
	bool drawFirst = vl::getAttribBool(XMLNode, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
	pElement = XMLNode->first_node("rotation");
	if( pElement )
	{ rotation = vl::parseQuaternion(pElement); }

	// Setup the sky box
	_scene_mgr->setSkyBox(true, material, distance, drawFirst, rotation );
}

void
vl::ogre::DotSceneLoader::processSkyDome(rapidxml::xml_node<>* XMLNode)
{
	// Process attributes
	// material attribute is required, all others are optional and have defaults
	std::string material = XMLNode->first_attribute("material")->value();
	Ogre::Real curvature = vl::getAttribReal(XMLNode, "curvature", 10);
	Ogre::Real tiling = vl::getAttribReal(XMLNode, "tiling", 8);
	Ogre::Real distance = vl::getAttribReal(XMLNode, "distance", 4000);
	bool drawFirst = vl::getAttribBool(XMLNode, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
	pElement = XMLNode->first_node("rotation");
	if(pElement)
	{ rotation = vl::parseQuaternion(pElement); }

	// Setup the sky dome
	_scene_mgr->setSkyDome( true, material, curvature, tiling,
			distance, drawFirst, rotation, 16, 16, -1 );

	std::string message = "Skydome Created with material " + material;
	Ogre::LogManager::getSingleton().logMessage( message );
}

void
vl::ogre::DotSceneLoader::processSkyPlane(rapidxml::xml_node<>* XMLNode)
{
//	TODO implement SkyPlane
	// Process attributes
	std::string material = vl::getAttrib(XMLNode, "material");
	Ogre::Real planeX = vl::getAttribReal(XMLNode, "planeX", 0);
	Ogre::Real planeY = vl::getAttribReal(XMLNode, "planeY", -1);
	Ogre::Real planeZ = vl::getAttribReal(XMLNode, "planeX", 0);
	Ogre::Real planeD = vl::getAttribReal(XMLNode, "planeD", 5000);
	Ogre::Real scale = vl::getAttribReal(XMLNode, "scale", 1000);
	Ogre::Real bow = vl::getAttribReal(XMLNode, "bow", 0);
	Ogre::Real tiling = vl::getAttribReal(XMLNode, "tiling", 10);
	bool drawFirst = vl::getAttribBool(XMLNode, "drawFirst", true);

	// Setup the sky plane
	Ogre::Plane plane;
	plane.normal = Ogre::Vector3(planeX, planeY, planeZ);
	plane.d = planeD;
	_scene_mgr->setSkyPlane( true, plane, material, scale, tiling,
			drawFirst, bow, 1, 1 );
}
