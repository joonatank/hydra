/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#ifndef VL_OGRE_DOT_SCENELOADER_HPP
#define VL_OGRE_DOT_SCENELOADER_HPP

// Standard library includes
#include <vector>
#include <string>

// Own includes
#include "base/rapidxml.hpp"
#include "base/filesystem.hpp"

#include "resource.hpp"

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

namespace vl
{

namespace ogre
{

class nodeProperty
{
public:
	std::string nodeName;
	std::string propertyNm;
	std::string valueName;
	std::string typeName;

	nodeProperty( const std::string &node,
			const std::string &propertyName,
			const std::string &value,
			const std::string &type )
		: nodeName(node),
		  propertyNm(propertyName),
		  valueName(value),
		  typeName(type)
	{}

};

/**	@class DotSceneLoader
 *	@brief Loads an dotscene file to the Ogre SceneManager
 */
class DotSceneLoader
{
public:
	DotSceneLoader( void );
	virtual ~DotSceneLoader( void );

	/** @brief Load dotscene file using Ogre resource system
	 @Param scene_name is the name of the scene file in Ogre Resource system
	 @param groupName is the name of the Ogre resource group
	 @param attachNode is the parent node for this DotScene scene
	 @param sPrependNode is a string which is added to the name of every node
	 *
	 *	Throws if there is an error.
	 *	If the passed scene_name is not found in Ogre Resource System
	 *	If the passed scene_name does not contain a valid dotscene file.
	 */
	void parseDotScene( std::string const &scene_name,
			Ogre::SceneManager *sceneMgr,
			Ogre::SceneNode *attachNode = 0,
			std::string const &sPrependNode = std::string() );

	std::string getProperty(const std::string &ndNm, const std::string &prop);

	std::vector<nodeProperty> nodeProperties;
	std::vector<std::string> staticObjects;
	std::vector<std::string> dynamicObjects;

protected:
	void _parse( char *xml_data );

	void processScene( rapidxml::xml_node<>* XMLRoot );

	void processNodes( rapidxml::xml_node<>* XMLNode );
	void processEnvironment( rapidxml::xml_node<>* XMLNode );
	void processTerrain( rapidxml::xml_node<>* XMLNode );
	void processTerrainPage( rapidxml::xml_node<>* XMLNode );
	void processLight( rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent = 0 );
	void processCamera( rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent  = 0 );

	void processNode(rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent = 0 );
	void processLookTarget(rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent);
	void processTrackTarget(rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent);
	void processParticleSystem(rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent);
	void processBillboardSet(rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent);
	void processPlane(rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent);

	void processFog( rapidxml::xml_node<>* XMLNode );
	void processSkyBox( rapidxml::xml_node<>* XMLNode );
	void processSkyDome( rapidxml::xml_node<>* XMLNode );
	void processSkyPlane( rapidxml::xml_node<>* XMLNode );
	void processClipping( rapidxml::xml_node<>* XMLNode );

	void processLightRange(rapidxml::xml_node<>* XMLNode,
			Ogre::Light *light );
	void processLightAttenuation( rapidxml::xml_node<>* XMLNode,
			Ogre::Light *light );

	// Scene Manager for the scene file
	Ogre::SceneManager *_scene_mgr;
	// Parent SceneNode for the scene file
	Ogre::SceneNode *_attach_node;

	std::string _sPrependNode;

};	// class DotSceneLoader

}	// namespace ogre

}	// namespace vl

#endif // VL_OGRE_DOT_SCENELOADER_HPP
