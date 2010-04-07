#ifndef DOT_SCENELOADER_H
#define DOT_SCENELOADER_H

// Includes
// OGre includes should be removed
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreResourceGroupManager.h>

// Standard library includes
#include <vector>
#include <string>

// Own includes
#include "base/rapidxml.hpp"
#include "base/typedefs.hpp"
#include "math/math.hpp"

// Forward declarations
/*
namespace Ogre
{
	class SceneManager;
	class SceneNode;
	class TerrainGroup;
	class TerrainGlobalOptions;
}
*/


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
//	Ogre::TerrainGlobalOptions *mTerrainGlobalOptions;
	
	DotSceneLoader();
	virtual ~DotSceneLoader();

	void parseDotScene( const std::string &SceneName,
			const std::string &groupName,
			vl::graph::SceneManagerRefPtr sceneMgr,
			vl::graph::SceneNodeRefPtr attachNode = vl::graph::SceneNodeRefPtr() ,
			const std::string &sPrependNode = "" );

	std::string getProperty(const std::string &ndNm, const std::string &prop);

//	Ogre::TerrainGroup* getTerrainGroup() { return mTerrainGroup; }

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
			vl::graph::SceneNodeRefPtr parent = vl::graph::SceneNodeRefPtr() );
	void processUserDataReference( rapidxml::xml_node<>* XMLNode,
			vl::graph::EntityRefPtr entity );
	void processOctree( rapidxml::xml_node<>* XMLNode );
	void processLight( rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent = vl::graph::SceneNodeRefPtr() );
	void processCamera( rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent  = vl::graph::SceneNodeRefPtr() );

	void processNode(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNode parent = vl::graph::SceneNodeRefPtr() );
	void processLookTarget(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent);
	void processTrackTarget(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent);
	void processEntity(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent);
	void processParticleSystem(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent);
	void processBillboardSet(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent);
	void processPlane(rapidxml::xml_node<>* XMLNode,
			vl::graph::SceneNodeRefPtr parent);

	void processFog( rapidxml::xml_node<>* XMLNode );
	void processSkyBox( rapidxml::xml_node<>* XMLNode );
	void processSkyDome( rapidxml::xml_node<>* XMLNode );
	void processSkyPlane( rapidxml::xml_node<>* XMLNode );
	void processClipping( rapidxml::xml_node<>* XMLNode );

	void processLightRange(rapidxml::xml_node<>* XMLNode,
			vl::graph::LightRefPtr light );
	void processLightAttenuation( rapidxml::xml_node<>* XMLNode,
			vl::graph::LightRefPtr light );

	std::string getAttrib(rapidxml::xml_node<>* XMLNode,
			const std::string &parameter,
			const std::string &defaultValue = "");

	vl::scalar getAttribReal(rapidxml::xml_node<>* XMLNode,
			const std::string &parameter, vl::scalar defaultValue = 0);

	bool getAttribBool(rapidxml::xml_node<>* XMLNode,
			const std::string &parameter, bool defaultValue = false);

	vl::vector parseVector3(rapidxml::xml_node<>* XMLNode);
	vl::quaternion parseQuaternion(rapidxml::xml_node<>* XMLNode);
	vl::colour parseColour(rapidxml::xml_node<>* XMLNode);
	
	// Scene Manager for the scene file
	vl::graph::SceneManagerRefPtr _scene_mgr;
	// Parent SceneNode for the scene file
	vl::graph::SceneNodeRefPtr _attach_node;

	// Ogre Resource group Name
	std::string m_sGroupName;

	std::string m_sPrependNode;

	// No terrain support for now
//	Ogre::TerrainGroup* mTerrainGroup;
//	Ogre::Vector3 mTerrainPosition;

	// Eh what the 
	vl::vector mLightDirection;
};

#endif // DOT_SCENELOADER_H