/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	DotScene file loader for distributed objects. Creates the object in config.
 *	So that they can be retrieved with getSceneNode later.
 *
 *	Does NOT support any other types than SceneNodes for now.
 *	Also does NOT support anything else from SceneNode than name and transformation.
 */

#ifndef VL_DOTSCENE_LOADER_HPP
#define VL_DOTSCENE_LOADER_HPP


#include <string>

#include "base/rapidxml.hpp"

#include "scene_node.hpp"
#include "config.hpp"

namespace vl
{

class DotSceneLoader
{
public :
	DotSceneLoader();

	virtual ~DotSceneLoader();

	/// Load dotscene file using Ogre resource system
	/// Parameters : sceneName is the name of the scene file in Ogre Resource system
	///				 groupName is the name of the Ogre resource group
	///				 attachNode is the parent node for this DotScene scene
	///				 sPrependNode is a string which is added to the name of every node
	void parseDotScene( std::string const &sceneName,
			std::string const &groupName,
			eqOgre::Config *config,
			eqOgre::SceneNode *attachNode = 0,
			std::string const &sPrependNode = std::string() );


	/// Parse dotscene which is already loaded to scene_data
	/// Parameters : scene_data is complete xml file loaded into a string
	void parseDotScene( std::string const &scene_data,
			eqOgre::Config *config,
			eqOgre::SceneNode *attachNode = 0,
			std::string const &sPrependNode = std::string() );

private :
	void processScene( rapidxml::xml_node<>* XMLRoot );

	void processNodes( rapidxml::xml_node<>* XMLNode );

	void processNode(rapidxml::xml_node<>* XMLNode,
			eqOgre::SceneNode *parent = 0 );


	char *_xml_data;

	eqOgre::Config *_config;

	std::string _sPrependNode;

};	// class DotSceneLoader

}	// namespace vl


#endif