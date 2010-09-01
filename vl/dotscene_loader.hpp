#ifndef VL_DOT_SCENELOADER_H
#define VL_DOT_SCENELOADER_H

// Standard library includes
#include <vector>
#include <string>

// Own includes
#include "base/rapidxml.hpp"
#include "base/filesystem.hpp"

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

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

class DotSceneLoader
{
public:
	DotSceneLoader();
	virtual ~DotSceneLoader();

	// Load dotscene file using Ogre resource system
	void parseDotScene( std::string const &sceneName,
			std::string const &groupName,
			Ogre::SceneManager *sceneMgr,
			Ogre::SceneNode *attachNode = 0,
			std::string const &sPrependNode = std::string() );

	// Parse dotscene which is already loaded to scene_data
	void parseDotScene( std::string const &scene_data,
			Ogre::SceneManager *sceneMgr,
			Ogre::SceneNode *attachNode = 0,
			std::string const &sPrependNode = std::string() );

	std::string getProperty(const std::string &ndNm, const std::string &prop);

	std::vector<nodeProperty> nodeProperties;
	std::vector<std::string> staticObjects;
	std::vector<std::string> dynamicObjects;

protected:
	void processScene( rapidxml::xml_node<>* XMLRoot );

	void processNodes( rapidxml::xml_node<>* XMLNode );
	void processExternals( rapidxml::xml_node<>* XMLNode );
	void processEnvironment( rapidxml::xml_node<>* XMLNode );
	void processTerrain( rapidxml::xml_node<>* XMLNode );
	void processTerrainPage( rapidxml::xml_node<>* XMLNode );
	void processBlendmaps( rapidxml::xml_node<>* XMLNode );
	void processUserDataReference( rapidxml::xml_node<>* XMLNode,
			Ogre::SceneNode *parent = 0 );
	void processUserDataReference( rapidxml::xml_node<>* XMLNode,
			Ogre::Entity *entity );
	void processOctree( rapidxml::xml_node<>* XMLNode );
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
	void processEntity(rapidxml::xml_node<>* XMLNode,
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

	std::string getAttrib(rapidxml::xml_node<>* XMLNode,
			const std::string &parameter,
			const std::string &defaultValue = "");

	Ogre::Real getAttribReal(rapidxml::xml_node<>* XMLNode,
			const std::string &parameter, Ogre::Real defaultValue = 0);

	bool getAttribBool(rapidxml::xml_node<>* XMLNode,
			const std::string &parameter, bool defaultValue = false);

	Ogre::Vector3 parseVector3(rapidxml::xml_node<>* XMLNode);
	Ogre::Quaternion parseQuaternion(rapidxml::xml_node<>* XMLNode);
	Ogre::ColourValue parseColour(rapidxml::xml_node<>* XMLNode);
	
	// Scene Manager for the scene file
	Ogre::SceneManager *_scene_mgr;
	// Parent SceneNode for the scene file
	Ogre::SceneNode *_attach_node;

	// Ogre Resource group Name
	std::string _sGroupName;

	std::string _sPrependNode;

	char *_xml_data;
	// No terrain support for now
//	Ogre::TerrainGroup* mTerrainGroup;
//	Ogre::Vector3 mTerrainPosition;

	// Eh what the 
//	Ogre::Vector3 mLightDirection;
};

#endif // DOT_SCENELOADER_H
