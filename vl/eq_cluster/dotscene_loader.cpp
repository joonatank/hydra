/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include "dotscene_loader.hpp"

// Necessary for logggin
#include <OGRE/OgreLogManager.h>

#include <OGRE/OgreException.h>
#include <OGRE/OgreResourceManager.h>

#include "base/string_utils.hpp"
#include "ogre_xml_helpers.hpp"

vl::DotSceneLoader::DotSceneLoader()
	: _xml_data(0)
{

}

vl::DotSceneLoader::~DotSceneLoader()
{
	delete [] _xml_data;
}


void
vl::DotSceneLoader::parseDotScene( 	const std::string& sceneName,
									const std::string& groupName,
									eqOgre::Config *config,
									eqOgre::SceneNode* attachNode,
									const std::string& sPrependNode )
{
	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton()
		.openResource( sceneName, groupName );

	parseDotScene( stream->getAsString(), config, attachNode, sPrependNode );
}

void
vl::DotSceneLoader::parseDotScene( const std::string& scene_data,
								   eqOgre::Config *config,
								   eqOgre::SceneNode* attachNode,
								   const std::string& sPrependNode )
{
	// set up shared object values
	_config = config;
	_sPrependNode = sPrependNode;

	rapidxml::xml_document<> XMLDoc;    // character type defaults to char

	rapidxml::xml_node<>* XMLRoot;

	delete [] _xml_data;
	_xml_data = new char[scene_data.length()+1];
	::strcpy( _xml_data, scene_data.c_str() );

	XMLDoc.parse<0>( _xml_data );

	// Grab the scene node
	XMLRoot = XMLDoc.first_node("scene");

	// Validate the File
	if( getAttrib(XMLRoot, "formatVersion", "") == "")
	{
		std::string message("[DotSceneLoader] Error: Invalid .scene File. Missing <scene>" );
		//	TODO add logging
		Ogre::LogManager::getSingleton().logMessage( message );
		// TODO add description
		BOOST_THROW_EXCEPTION( vl::invalid_dotscene() );
	}

	// Process the scene
	processScene(XMLRoot);
}



/// ------- DotSceneLoader Private -------------
void
vl::DotSceneLoader::processScene(rapidxml::xml_node<  >* XMLRoot)
{
	rapidxml::xml_node<>* pElement;

	// Process nodes (?)
	pElement = XMLRoot->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }
}

void
vl::DotSceneLoader::processNodes(rapidxml::xml_node<  >* XMLNode)
{
	rapidxml::xml_node<>* pElement;

	// Process node (*)
	pElement = XMLNode->first_node("node");
	while(pElement)
	{
		processNode(pElement);
		pElement = pElement->next_sibling("node");
	}

}

void
vl::DotSceneLoader::processNode(rapidxml::xml_node<  >* XMLNode, eqOgre::SceneNode* parent)
{
	// Parents are not supported yet

	// Construct the node's name
	std::string name = _sPrependNode + vl::getAttrib(XMLNode, "name");

	// Create the scene node
	eqOgre::SceneNode *node = _config->createSceneNode(name);

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if( pElement )
	{ node->setPosition(vl::parseVector3(pElement)); }

	// Process rotation (?)
	pElement = XMLNode->first_node("rotation");
	if( pElement )
	{ node->setOrientation(vl::parseQuaternion(pElement)); }

	pElement = XMLNode->first_node("quaternion");
	if( pElement )
	{ node->setOrientation(vl::parseQuaternion(pElement)); }
}
