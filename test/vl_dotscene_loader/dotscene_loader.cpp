#include "dotscene_loader.hpp"

// Ogre includes TODO remove
#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreResourceManager.h>
/*
#include <Ogre.h>
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include <Terrain/OgreTerrainMaterialGeneratorA.h>
*/

#include <sstream>

#include "base/string_utils.hpp"
#include "interface/scene_node.hpp"
#include "interface/scene_manager.hpp"
#include "interface/entity.hpp"
#include "interface/light.hpp"
#include "interface/camera.hpp"

/* For now we are using Ogre::LogManager and Ogre::ResourceGroupManager
 * Because we don't have our own logging or resource system in place.
 * TODO should be removed as soon as possible.
 */

DotSceneLoader::DotSceneLoader()
{
}


DotSceneLoader::~DotSceneLoader()
{
}

void
DotSceneLoader::parseDotScene(
		const std::string &SceneName,
		const std::string &groupName,
		vl::graph::SceneManagerRefPtr sceneMgr,
		vl::graph::SceneNodeRefPtr attachNode,
		const std::string &sPrependNode )
{
	// set up shared object values
	m_sGroupName = groupName;
	_scene_mgr = sceneMgr;
	m_sPrependNode = sPrependNode;
	staticObjects.clear();
	dynamicObjects.clear();

	rapidxml::xml_document<> XMLDoc;    // character type defaults to char

	rapidxml::xml_node<>* XMLRoot;

	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton()
		.openResource(SceneName, groupName );
	char* scene = strdup(stream->getAsString().c_str());
	XMLDoc.parse<0>(scene);

	// Grab the scene node
	XMLRoot = XMLDoc.first_node("scene");

	// Validate the File
	if( getAttrib(XMLRoot, "formatVersion", "") == "")
	{
		std::string message("[DotSceneLoader] Error: Invalid .scene File. Missing <scene>" );
		Ogre::LogManager::getSingleton().logMessage( message );
		return;
	}

	// figure out where to attach any nodes we create
	_attach_node = pAttachNode;
	if( !_attach_node )
	{ _attach_node = mSceneMgr->getRootSceneNode(); }

	// Process the scene
	processScene(XMLRoot);
}

void DotSceneLoader::processScene(rapidxml::xml_node<>* XMLRoot)
{
	// Process the scene parameters
	std::string version = getAttrib(XMLRoot, "formatVersion", "unknown");

	std::string message = "[DotSceneLoader] Parsing dotScene file with version " + version;
	if(XMLRoot->first_attribute("ID"))
	{
		message += ", id " + std::string(XMLRoot->first_attribute("ID")->value());
	}
	if(XMLRoot->first_attribute("sceneManager"))
	{
		message += ", scene manager "
			+ std::string(XMLRoot->first_attribute("sceneManager")->value());
	}
	if(XMLRoot->first_attribute("minOgreVersion"))
	{
		message += ", min. Ogre version "
			+ std::string(XMLRoot->first_attribute("minOgreVersion")->value());
	}
	if(XMLRoot->first_attribute("author"))
	{
		message += ", author "
			+ std::string(XMLRoot->first_attribute("author")->value());
	}

	Ogre::LogManager::getSingleton().logMessage(message);

	rapidxml::xml_node<>* pElement;

	// Process environment (?)
	pElement = XMLRoot->first_node("environment");
	if(pElement)
	{ processEnvironment(pElement); }

	// Process nodes (?)
	pElement = XMLRoot->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }

	// Process externals (?)
	pElement = XMLRoot->first_node("externals");
	if(pElement)
	{ processExternals(pElement); }

	// Process userDataReference (?)
	pElement = XMLRoot->first_node("userDataReference");
	if(pElement)
	{ processUserDataReference(pElement); }

	// Process octree (?)
	pElement = XMLRoot->first_node("octree");
	if(pElement)
	{ processOctree(pElement); }

	// Process light (?)
	//pElement = XMLRoot->first_node("light");
	//if(pElement)
	//	processLight(pElement);

	// Process camera (?)
	pElement = XMLRoot->first_node("camera");
	if(pElement)
	{ processCamera(pElement); }

	// Process terrain (?)
	pElement = XMLRoot->first_node("terrain");
	if(pElement)
	{ processTerrain(pElement); }
}

void DotSceneLoader::processNodes(rapidxml::xml_node<>* XMLNode)
{
	rapidxml::xml_node<>* pElement;

	// Process node (*)
	pElement = XMLNode->first_node("node");
	while(pElement)
	{
		processNode(pElement);
		pElement = pElement->next_sibling("node");
	}

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if(pElement)
	{
		_attach_node->setPosition(parseVector3(pElement));
		_attach_node->setInitialState();
	}

	// Process rotation (?)
	pElement = XMLNode->first_node("rotation");
	if(pElement)
	{
		_attach_node->setOrientation(parseQuaternion(pElement));
		_attach_node->setInitialState();
	}

	// Process scale (?)
	pElement = XMLNode->first_node("scale");
	if(pElement)
	{
		_attach_node->setScale(parseVector3(pElement));
		_attach_node->setInitialState();
	}
}

void DotSceneLoader::processExternals(rapidxml::xml_node<>* XMLNode)
{
	//! @todo Implement this
}

void DotSceneLoader::processEnvironment(rapidxml::xml_node<>* XMLNode)
{
	rapidxml::xml_node<>* pElement;

	// Process camera (?)
	pElement = XMLNode->first_node("camera");
	if(pElement)
	{ processCamera(pElement); }

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

	// Process clipping (?)
	pElement = XMLNode->first_node("clipping");
	if( pElement )
	{ processClipping(pElement); }

	// Process colourAmbient (?)
	pElement = XMLNode->first_node("colourAmbient");
	if( pElement )
	{ mSceneMgr->setAmbientLight(parseColour(pElement)); }

	// Process colourBackground (?)
	//! @todo Set the background colour of all viewports (RenderWindow has to be provided then)
	pElement = XMLNode->first_node("colourBackground");
	if( pElement )
		;//mSceneMgr->set(parseColour(pElement));

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if( pElement )
	{ processUserDataReference(pElement); }
}

void
DotSceneLoader::processTerrain(rapidxml::xml_node<>* XMLNode)
{
	/*	Terrain not yet Implemented
	vl::scalar worldSize = getAttribReal(XMLNode, "worldSize");
	int mapSize = std::stringConverter::parseInt(XMLNode->first_attribute("mapSize")->value());
	//	Not used
	int rows = std::stringConverter::parseInt(XMLNode->first_attribute("rows")->value());
	int columns = std::stringConverter::parseInt(XMLNode->first_attribute("columns")->value());
	bool colourmapEnabled = getAttribBool(XMLNode, "colourmapEnabled");
	int colourMapTextureSize = std::stringConverter::
			parseInt(XMLNode->first_attribute("colourMapTextureSize")->value());

	vl::vector3 lightdir(0, -0.3, 0.75);
	lightdir.normalise();
	vl::graph::LightRefPtr* l = mSceneMgr->createLight("tstLight");
	l->setType(vl::graph::LightRefPtr::LT_DIRECTIONAL);
	l->setDirection(lightdir);
	l->setDiffuseColour(vl::ColourValue(1.0, 1.0, 1.0));
	l->setSpecularColour(vl::ColourValue(0.4, 0.4, 0.4));
	mSceneMgr->setAmbientLight(vl::ColourValue(0.6, 0.6, 0.6));

	mTerrainGlobalOptions->setMaxPixelError(1);
	mTerrainGlobalOptions->setCompositeMapDistance(2000);
	mTerrainGlobalOptions->setLightMapDirection(lightdir);
	mTerrainGlobalOptions->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobalOptions->setCompositeMapDiffuse(l->getDiffuseColour());

	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneMgr,
			Ogre::Terrain::ALIGN_X_Z, mapSize, worldSize);
	mTerrainGroup->setOrigin(vl::vector3::ZERO);

	rapidxml::xml_node<>* pElement;
	rapidxml::xml_node<>* pPageElement;

	// Process terrain pages (*)
	pElement = XMLNode->first_node("terrainPages");
	while(pElement)
	{
		pPageElement = pElement->first_node("terrainPage");
		while(pPageElement)
		{
			processTerrainPage(pPageElement);
			pPageElement = pPageElement->next_sibling("terrainPage");
		}
		pElement = pElement->next_sibling("terrainPages");
	}
	mTerrainGroup->loadAllTerrains(true);

	// process blendmaps
	pElement = XMLNode->first_node("terrainPages");
	while(pElement)
	{
		pPageElement = pElement->first_node("terrainPage");
		while(pPageElement)
		{
			processBlendmaps(pPageElement);
			pPageElement = pPageElement->next_sibling("terrainPage");
		}
		pElement = pElement->next_sibling("terrainPages");
	}
	mTerrainGroup->freeTemporaryResources();
	//mTerrain->setPosition(mTerrainPosition);
	*/
}

void
DotSceneLoader::processTerrainPage(rapidxml::xml_node<>* XMLNode)
{
	/*
	std::string name = getAttrib(XMLNode, "name");
	int pageX = std::stringConverter::parseInt(XMLNode->first_attribute("pageX")->value());
	int pageY = std::stringConverter::parseInt(XMLNode->first_attribute("pageY")->value());
	vl::scalar worldSize = getAttribReal(XMLNode, "worldSize");

	size_t mapSize; //= std::stringConverter::parseInt(
	std::stringstream ss( std::stringstream::in | std::stringstream::out );
	ss << XMLNode->first_attribute("mapSize")->value();
	ss >> mapSize;

	bool colourmapEnabled = getAttribBool(XMLNode, "colourmapEnabled");
	int colourmapTexturesize = std::stringConverter::parseInt(
			XMLNode->first_attribute("colourmapTexturesize")->value());
	int layerCount = std::stringConverter::parseInt(XMLNode->first_attribute("layerCount")->value());

	std::string filename = mTerrainGroup->generateFilename(pageX, pageY);
	if( Ogre::ResourceGroupManager::getSingleton()
			.resourceExists(mTerrainGroup->getResourceGroup(), filename ))
	{ mTerrainGroup->defineTerrain(pageX, pageY); }
	else
	{
		rapidxml::xml_node<>* pElement;

		pElement = XMLNode->first_node("position");
		if(pElement)
			mTerrainPosition = parseVector3(pElement);

		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton()
			.openResource(name + std::string(".ohm"), "General" );
		size_t size = stream.get()->size();
		if(size != mapSize * mapSize * 4)
		{
			OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR,
					"Size of stream does not match terrainsize!", "TerrainPage" );
		}
		float* buffer = OGRE_ALLOC_T(float, size, Ogre::MEMCATEGORY_GEOMETRY);
		stream->read(buffer, size);

		Ogre::Terrain::ImportData& imp = mTerrainGroup->getDefaultImportSettings();
		imp.terrainSize = mapSize;
		imp.worldSize = worldSize;
		imp.inputFloat = buffer;
		imp.inputImage = 0;
		imp.deleteInputData = true;
		imp.minBatchSize = 33;
		imp.maxBatchSize = 65;

		imp.layerList.resize(layerCount);

		int count = 0;

		// Process layers (*)
		rapidxml::xml_node<>* pTerrainLayer;
		rapidxml::xml_node<>* pTerrainTextures;
		rapidxml::xml_node<>* pTerrainTexture;
		pElement = XMLNode->first_node("layers");
		while(pElement)
		{
			pTerrainLayer = pElement->first_node("layer");
			while(pTerrainLayer)
			{
				int worldSize = std::stringConverter::parseInt(
						pTerrainLayer->first_attribute("worldSize")->value());
				pTerrainTextures = pTerrainLayer->first_node("textures");
				pTerrainTexture = pTerrainTextures->first_node("texture");
				while(pTerrainTexture)
				{
					imp.layerList[count].textureNames.push_back(getAttrib(
								pTerrainTexture,"name",""));
                    imp.layerList[count].worldSize = (vl::scalar)worldSize;
					pTerrainTexture = pTerrainTexture->next_sibling("texture");
				}
				count++;
				// do stuff
				pTerrainLayer = pTerrainLayer->next_sibling("layer");
			}
			pElement = pElement->next_sibling("layers");
		}
		
		mTerrainGroup->defineTerrain(pageX, pageY, &imp);
	}
	*/
}

void
DotSceneLoader::processBlendmaps(rapidxml::xml_node<>* XMLNode)
{
	/* TODO implement Terrain
	int pageX = std::stringConverter::parseInt(
			XMLNode->first_attribute("pageX")->value());
	int pageY = std::stringConverter::parseInt(
			XMLNode->first_attribute("pageY")->value());

	std::string filename = mTerrainGroup->generateFilename(pageX, pageY);
	// skip this is terrain page has been saved already
	if( !Ogre::ResourceGroupManager::getSingleton()
			.resourceExists(mTerrainGroup->getResourceGroup(), filename) )
	{
		rapidxml::xml_node<>* pElement;

		// Process blendmaps (*)
		std::vector<std::string> blendMaps;
		rapidxml::xml_node<>* pBlendmap;
		pElement = XMLNode->first_node("blendMaps");
		pBlendmap = pElement->first_node("blendMap");
		while(pBlendmap)
		{
			blendMaps.push_back(getAttrib(pBlendmap, "texture",""));
			pBlendmap = pBlendmap->next_sibling("blendMap");
		}
		int layerCount = mTerrainGroup->getTerrain(pageX, pageY)->getLayerCount();
		for( int j = 1; j < layerCount; j++ )
		{
			Ogre::TerrainLayerBlendMap *blendmap
				= mTerrainGroup->getTerrain(pageX, pageY)->getLayerBlendMap(j);
			Ogre::Image img;
			img.load(blendMaps[j-1],"General");
			int blendmapsize = mTerrainGroup->getTerrain(pageX, pageY)->getLayerBlendMapSize();
			if( img.getWidth() != blendmapsize )
			{ img.resize(blendmapsize, blendmapsize); }

			float *ptr = blendmap->getBlendPointer();
			Ogre::uint8 *data = static_cast<Ogre::uint8*>(img.getPixelBox().data);

			for(int bp = 0;bp < blendmapsize * blendmapsize;bp++)
				ptr[bp] = static_cast<float>(data[bp]) / 255.0f;

			blendmap->dirty();
			blendmap->update();
		}
	}
	*/
}

void
DotSceneLoader::processUserDataReference(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	//! @todo Implement this
}

void
DotSceneLoader::processOctree(rapidxml::xml_node<>* XMLNode)
{
	//! @todo Implement this
}

void
DotSceneLoader::processLight(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	// Process attributes
	std::string name = getAttrib(XMLNode, "name");
	std::string id = getAttrib(XMLNode, "id");

	// Create the light
	vl::graph::LightRefPtr light = mSceneMgr->createLight(name);
	if( parent )
	{ parent->attachObject(light); }

	std::string sValue = getAttrib(XMLNode, "type");
	if(sValue == "point")
	{ light->setType(vl::graph::Light::LT_POINT); }
	else if(sValue == "directional")
	{ light->setType(vl::graph::Light::LT_DIRECTIONAL); }
	else if(sValue == "spot")
	{ light->setType(vl::graph::Light::LT_SPOTLIGHT); }
	else if(sValue == "radPoint")
	{ light->setType(vl::graph::Light::LT_POINT); }

	light->setVisible(getAttribBool(XMLNode, "visible", true));
	light->setCastShadows(getAttribBool(XMLNode, "castShadows", true));

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if(pElement)
		light->setPosition(parseVector3(pElement));

	// Process normal (?)
	pElement = XMLNode->first_node("normal");
	if(pElement)
		light->setDirection(parseVector3(pElement));

	pElement = XMLNode->first_node("directionVector");
	if(pElement)
	{
		light->setDirection(parseVector3(pElement));
		mLightDirection = parseVector3(pElement);
	}

	// Process colourDiffuse (?)
	pElement = XMLNode->first_node("colourDiffuse");
	if(pElement)
		light->setDiffuseColour(parseColour(pElement));

	// Process colourSpecular (?)
	pElement = XMLNode->first_node("colourSpecular");
	if(pElement)
		light->setSpecularColour(parseColour(pElement));

	if(sValue != "directional")
	{
		// Process lightRange (?)
		pElement = XMLNode->first_node("lightRange");
		if(pElement)
			processLightRange(pElement, light);

		// Process lightAttenuation (?)
		pElement = XMLNode->first_node("lightAttenuation");
		if(pElement)
			processLightAttenuation(pElement, light);
	}
	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if(pElement)
		;//processUserDataReference(pElement, light);
}

void
DotSceneLoader::processCamera(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	if( !parent )
	{ throw vl::exception( "No parent", "DotSceneLoader::processCamera" ); }

	// Process attributes
	std::string name = getAttrib(XMLNode, "name");
	std::string id = getAttrib(XMLNode, "id");
	//	TODO not implemented
//	vl::scalar fov = getAttribReal(XMLNode, "fov", 45);
//	vl::scalar aspectRatio = getAttribReal(XMLNode, "aspectRatio", 1.3333);
	std::string projectionType = getAttrib(XMLNode, "projectionType", "perspective");

	std::string node_name = name + std::string("Node");
	// Create the camera
	vl::graph::CameraRefPtr camera = _scene_mgr->createCamera( name );
	vl::graph::SceneNodeRefPtr node = _scene_mgr->createSceneNode( node_name );
	node->attachObject( camera );
	parent->addChild( camera );

	// TODO: make a flag or attribute indicating whether or not the camera
	// should be attached to any parent node.

	// Set the field-of-view
	//! @todo Is this always in degrees?
	//camera->setFOVy(Ogre::Degree(fov));

	// Set the aspect ratio
	//camera->setAspectRatio(aspectRatio);
	
	// Set the projection type
	/*	TODO not implemented
	if(projectionType == "perspective")
	{ camera->setProjectionType(Ogre::PT_PERSPECTIVE); }
	else if(projectionType == "orthographic")
	{ camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC); }
	*/

	rapidxml::xml_node<>* pElement;

	// Process clipping (?)
	pElement = XMLNode->first_node("clipping");
	if(pElement)
	{
		vl::scalar nearDist = getAttribReal(pElement, "near");
		camera->setNearClipDistance(nearDist);

		vl::scalar farDist =  getAttribReal(pElement, "far");
		camera->setFarClipDistance(farDist);
	}

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if(pElement)
	{ camera->setPosition(parseVector3(pElement)); }

	// Process rotation (?)
	pElement = XMLNode->first_node("rotation");
	if(pElement)
	{ camera->setOrientation(parseQuaternion(pElement)); }

	// Process normal (?)
	pElement = XMLNode->first_node("normal");
	if(pElement)
		;//!< @todo What to do with this element?

	// Process lookTarget (?)
	pElement = XMLNode->first_node("lookTarget");
	if(pElement)
		;//!< @todo Implement the camera look target

	// Process trackTarget (?)
	pElement = XMLNode->first_node("trackTarget");
	if(pElement)
		;//!< @todo Implement the camera track target

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if(pElement)
		;//!< @todo Implement the camera user data reference

	// construct a scenenode is no parent
	/*
	if( !parent )
	{
		vl::graph::SceneNodeRefPtr* node = _attach_node->createChildSceneNode(name);
		node->setPosition(camera->getPosition());
		node->setOrientation(camera->getOrientation());
		node->scale(1,1,1);
	}
	*/
}

void
DotSceneLoader::processNode(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	// Construct the node's name
	std::string name = m_sPrependNode + getAttrib(XMLNode, "name");

	// Create the scene node
	vl::graph::SceneNodeRefPtr node;

	// TODO this function should always be called with parent
	// so that if no other parent is then _attachNode becomes the parent.
	if(parent)
	{ node = parent->createChildSceneNode(name); }
	else
	{ node = _attach_node->createChildSceneNode(name); }

	// Process other attributes
	std::string id = getAttrib(XMLNode, "id");
	// Not used
//	bool isTarget = getAttribBool(XMLNode, "isTarget");

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if( pElement )
	{
		node->setPosition(parseVector3(pElement));
		node->setInitialState();
	}

	// Process rotation (?)
	pElement = XMLNode->first_node("rotation");
	if( pElement )
	{
		node->setOrientation(parseQuaternion(pElement));
		node->setInitialState();
	}

	// Process scale (?)
	pElement = XMLNode->first_node("scale");
	if(pElement)
	{
		node->setScale(parseVector3(pElement));
		node->setInitialState();
	}

	// Process lookTarget (?)
	pElement = XMLNode->first_node("lookTarget");
	if(pElement)
	{ processLookTarget(pElement, node); }

	// Process trackTarget (?)
	pElement = XMLNode->first_node("trackTarget");
	if(pElement)
		processTrackTarget(pElement, node);

	// Process node (*)
	pElement = XMLNode->first_node("node");
	while(pElement)
	{
		processNode(pElement, node);
		pElement = pElement->next_sibling("node");
	}

	// Process entity (*)
	pElement = XMLNode->first_node("entity");
	while(pElement)
	{
		processEntity(pElement, node);
		pElement = pElement->next_sibling("entity");
	}

	// Process light (*)
	//pElement = XMLNode->first_node("light");
	//while(pElement)
	//{
	//	processLight(pElement, node);
	//	pElement = pElement->next_sibling("light");
	//}

	// Process camera (*)
	pElement = XMLNode->first_node("camera");
	while(pElement)
	{
		processCamera(pElement, node);
		pElement = pElement->next_sibling("camera");
	}

	// Process particleSystem (*)
	pElement = XMLNode->first_node("particleSystem");
	while(pElement)
	{
		processParticleSystem(pElement, node);
		pElement = pElement->next_sibling("particleSystem");
	}

	// Process billboardSet (*)
	pElement = XMLNode->first_node("billboardSet");
	while( pElement )
	{
		processBillboardSet(pElement, node);
		pElement = pElement->next_sibling("billboardSet");
	}

	// Process plane (*)
	pElement = XMLNode->first_node("plane");
	while( pElement )
	{
		processPlane(pElement, node);
		pElement = pElement->next_sibling("plane");
	}

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if( pElement )
	{ processUserDataReference(pElement, node); }
}

void
DotSceneLoader::processLookTarget(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	//! @todo Is this correct? Cause I don't have a clue actually
	/*	TODO this needs SceneManager find by name and SceneNode::lookAt

	// Process attributes
	std::string nodeName = getAttrib(XMLNode, "nodeName");

	vl::graph::SceneNode::TransformSpace relativeTo
		= vl::graph::SceneNode::TS_PARENT;
	std::string sValue = getAttrib(XMLNode, "relativeTo");
	if(sValue == "local")
		relativeTo = vl::graph::SceneNode::TS_LOCAL;
	else if(sValue == "parent")
		relativeTo = vl::graph::SceneNode::TS_PARENT;
	else if(sValue == "world")
		relativeTo = vl::graph::SceneNode::TS_WORLD;

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	vl::vector position;
	pElement = XMLNode->first_node("position");
	if(pElement)
		position = parseVector3(pElement);

	// Process localDirection (?)
	vl::vector localDirection = vl::vector::NEGATIVE_UNIT_Z;
	pElement = XMLNode->first_node("localDirection");
	if(pElement)
		localDirection = parseVector3(pElement);

	// Setup the look target
	try
	{
		if( !nodeName.empty() )
		{
			vl::graph::SceneNodeRefPtr pLookNode = mSceneMgr->getSceneNode(nodeName);
			position = pLookNode->_getDerivedPosition();
		}

		parent->lookAt(position, relativeTo, localDirection);
	}
	catch( Ogre::Exception & )
	{
		std::string msg("[DotSceneLoader] Error processing a look target!");
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
	*/
}

void
DotSceneLoader::processTrackTarget(rapidxml::xml_node<>* XMLNode, vl::graph::SceneNodeRefPtr parent)
{
	/*	TODO this needs SceneNode::autoTracking
	 *
	// Process attributes
	std::string nodeName = getAttrib(XMLNode, "nodeName");

	rapidxml::xml_node<>* pElement;

	// Process localDirection (?)
	vl::vector localDirection = vl::vector::NEGATIVE_UNIT_Z;
	pElement = XMLNode->first_node("localDirection");
	if( pElement )
	{ localDirection = parseVector3(pElement); }

	// Process offset (?)
	vl::vector offset = vl::vector::ZERO;
	pElement = XMLNode->first_node("offset");
	if( pElement )
	{ offset = parseVector3(pElement); }

	// Setup the track target
	try
	{
		vl::graph::SceneNodeRefPtr *pTrackNode = mSceneMgr->getSceneNode(nodeName);
		parent->setAutoTracking(true, pTrackNode, localDirection, offset);
	}
	catch(Ogre::Exception & )
	{
		std::string msg("[DotSceneLoader] Error processing a track target!");
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
	*/
}

void
DotSceneLoader::processEntity(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	// TODO this needs SceneNode::attachObject,
	// Entity::setCastShadows, Entity::setMaterialName

	// Process attributes
	std::string name = getAttrib(XMLNode, "name");
	std::string id = getAttrib(XMLNode, "id");
	std::string meshFile = getAttrib(XMLNode, "meshFile");
	std::string materialFile = getAttrib(XMLNode, "materialFile");
	bool isStatic = getAttribBool(XMLNode, "static", false);;
	bool castShadows = getAttribBool(XMLNode, "castShadows", true);

	// TEMP: Maintain a list of static and dynamic objects
	if(isStatic)
	{ staticObjects.push_back(name); }
	else
	{ dynamicObjects.push_back(name); }

	rapidxml::xml_node<>* pElement;

	// Process vertexBuffer (?)
	pElement = XMLNode->first_node("vertexBuffer");
	if(pElement)
		;//processVertexBuffer(pElement);

	// Process indexBuffer (?)
	pElement = XMLNode->first_node("indexBuffer");
	if(pElement)
		;//processIndexBuffer(pElement);

	// Create the entity
	vl::graph::EntityRefPtr entity;
	try
	{
		//Ogre::MeshManager::getSingleton().load(meshFile, m_sGroupName);
		entity = _scene_mgr->createEntity(name, meshFile);
		entity->setCastShadows(castShadows);
		parent->attachObject(entity);
		
		if( !materialFile.empty() )
		{ entity->setMaterialName(materialFile); }
	}
	catch(Ogre::Exception &/*e*/)
	{
		std::string msg("[DotSceneLoader] Error loading an entity!");
		Ogre::LogManager::getSingleton().logMessage(msg);
	}

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if( pElement )
	{ processUserDataReference(pElement, entity); }
}

void
DotSceneLoader::processParticleSystem(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	/*	TODO this needs ParticleSystems
	// Process attributes
	std::string name = getAttrib(XMLNode, "name");
	std::string id = getAttrib(XMLNode, "id");
	std::string file = getAttrib(XMLNode, "file");

	// Create the particle system
	try
	{
		Ogre::ParticleSystem *pParticles
			= mSceneMgr->createParticleSystem(name, file);
		parent->attachObject(pParticles);
	}
	catch(Ogre::Exception & )
	{
		std::string msg("[DotSceneLoader] Error creating a particle system!");
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
	*/
}

void
DotSceneLoader::processBillboardSet(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
	//! @todo Implement this
}

void
DotSceneLoader::processPlane(rapidxml::xml_node<>* XMLNode,
		vl::graph::SceneNodeRefPtr parent)
{
/*	TODO implement plane
    std::string name = getAttrib(XMLNode, "name");
    vl::scalar distance = getAttribReal(XMLNode, "distance");
    vl::scalar width = getAttribReal(XMLNode, "width");
    vl::scalar height = getAttribReal(XMLNode, "height");
    int xSegments = std::stringConverter::parseInt(getAttrib(XMLNode, "xSegments"));
    int ySegments = std::stringConverter::parseInt(getAttrib(XMLNode, "ySegments"));
    int numTexCoordSets = std::stringConverter::parseInt(getAttrib(XMLNode, "numTexCoordSets"));
    vl::scalar uTile = getAttribReal(XMLNode, "uTile");
    vl::scalar vTile = getAttribReal(XMLNode, "vTile");
    std::string material = getAttrib(XMLNode, "material");
    bool hasNormals = getAttribBool(XMLNode, "hasNormals");
    vl::vector normal = parseVector3(XMLNode->first_node("normal"));
    vl::vector up = parseVector3(XMLNode->first_node("upVector"));

    Ogre::Plane plane(normal, distance);
    Ogre::MeshPtr res = Ogre::MeshManager::getSingletonPtr()->createPlane(
                        name + "mesh", "General",
						plane, width, height, xSegments, ySegments, hasNormals,
						numTexCoordSets, uTile, vTile, up );

    vl::graph::EntityRefPtr ent = _scene_mgr->createEntity(name, name + "mesh");

    ent->setMaterialName(material);

    parent->attachObject(ent);
*/
}

void
DotSceneLoader::processFog(rapidxml::xml_node<>* XMLNode)
{
/*	TODO implement Fog
	// Process attributes
	vl::scalar expDensity = getAttribReal(XMLNode, "density", 0.001);
	vl::scalar linearStart = getAttribReal(XMLNode, "start", 0.0);
	vl::scalar linearEnd = getAttribReal(XMLNode, "end", 1.0);

	Ogre::FogMode mode = Ogre::FOG_NONE;
	std::string sMode = getAttrib(XMLNode, "mode");
	if(sMode == "none")
		mode = Ogre::FOG_NONE;
	else if(sMode == "exp")
		mode = Ogre::FOG_EXP;
	else if(sMode == "exp2")
		mode = Ogre::FOG_EXP2;
	else if(sMode == "linear")
		mode = Ogre::FOG_LINEAR;

	rapidxml::xml_node<>* pElement;

	// Process colourDiffuse (?)
	vl::ColourValue colourDiffuse = vl::ColourValue::White;
	pElement = XMLNode->first_node("colour");
	if(pElement)
		colourDiffuse = parseColour(pElement);

	// Setup the fog
	mSceneMgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
*/
}

void
DotSceneLoader::processSkyBox(rapidxml::xml_node<>* XMLNode)
{
/*	TODO implement SkyBox
	// Process attributes
	std::string material = getAttrib(XMLNode, "material", "BaseWhite");
	vl::scalar distance = getAttribReal(XMLNode, "distance", 5000);
	bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
	bool active = getAttribBool(XMLNode, "active", false);
	if(!active)
		return;

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	vl::quaternion rotation = vl::quaternion::IDENTITY;
	pElement = XMLNode->first_node("rotation");
	if(pElement)
		rotation = parseQuaternion(pElement);

	// Setup the sky box
	mSceneMgr->setSkyBox(true, material, distance, drawFirst, rotation, m_sGroupName);
*/
}

void
DotSceneLoader::processSkyDome(rapidxml::xml_node<>* XMLNode)
{
/*	TODO implement SkyDome
	// Process attributes
	std::string material = XMLNode->first_attribute("material")->value();
	vl::scalar curvature = getAttribReal(XMLNode, "curvature", 10);
	vl::scalar tiling = getAttribReal(XMLNode, "tiling", 8);
	vl::scalar distance = getAttribReal(XMLNode, "distance", 4000);
	bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	vl::quaternion rotation = vl::quaternion::IDENTITY;
	pElement = XMLNode->first_node("rotation");
	if(pElement)
		rotation = parseQuaternion(pElement);

	// Setup the sky dome
	mSceneMgr->setSkyDome( true, material, curvature, tiling,
			distance, drawFirst, rotation, 16, 16, -1, m_sGroupName );
*/
}

void
DotSceneLoader::processSkyPlane(rapidxml::xml_node<>* XMLNode)
{
/*	TODO implement SkyPlane
	// Process attributes
	std::string material = getAttrib(XMLNode, "material");
	vl::scalar planeX = getAttribReal(XMLNode, "planeX", 0);
	vl::scalar planeY = getAttribReal(XMLNode, "planeY", -1);
	vl::scalar planeZ = getAttribReal(XMLNode, "planeX", 0);
	vl::scalar planeD = getAttribReal(XMLNode, "planeD", 5000);
	vl::scalar scale = getAttribReal(XMLNode, "scale", 1000);
	vl::scalar bow = getAttribReal(XMLNode, "bow", 0);
	vl::scalar tiling = getAttribReal(XMLNode, "tiling", 10);
	bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

	// Setup the sky plane
	Ogre::Plane plane;
	plane.normal = vl::vector3(planeX, planeY, planeZ);
	plane.d = planeD;
	mSceneMgr->setSkyPlane( true, plane, material, scale, tiling,
			drawFirst, bow, 1, 1, m_sGroupName );
*/
}

void
DotSceneLoader::processClipping(rapidxml::xml_node<>* /*XMLNode */)
{
	//! @todo Implement this

	// Process attributes
//	vl::scalar fNear = getAttribReal(XMLNode, "near", 0);
//	vl::scalar fFar = getAttribReal(XMLNode, "far", 1);
}

void
DotSceneLoader::processLightRange(rapidxml::xml_node<>* XMLNode, vl::graph::LightRefPtr light)
{
	// Process attributes
	vl::scalar inner = getAttribReal(XMLNode, "inner");
	vl::scalar outer = getAttribReal(XMLNode, "outer");
	vl::scalar falloff = getAttribReal(XMLNode, "falloff", 1.0);

	// Setup the light range
	light->setSpotlightRange(vl::angle(inner), vl::angle(outer), falloff);
}

void
DotSceneLoader::processLightAttenuation(rapidxml::xml_node<>* XMLNode,
		vl::graph::LightRefPtr light)
{
	// Process attributes
	vl::scalar range = getAttribReal(XMLNode, "range");
	vl::scalar constant = getAttribReal(XMLNode, "constant");
	vl::scalar linear = getAttribReal(XMLNode, "linear");
	vl::scalar quadratic = getAttribReal(XMLNode, "quadratic");

	// Setup the light attenuation
	light->setAttenuation(range, constant, linear, quadratic);
}


std::string
DotSceneLoader::getAttrib(rapidxml::xml_node<>* XMLNode,
		const std::string &attrib, const std::string &defaultValue )
{
	if(XMLNode->first_attribute(attrib.c_str()))
	{ return XMLNode->first_attribute(attrib.c_str())->value(); }
	else
	{ return defaultValue; }
}

vl::scalar
DotSceneLoader::getAttribReal( rapidxml::xml_node<>* XMLNode,
		const std::string &attrib, vl::scalar defaultValue )
{
	if(XMLNode->first_attribute(attrib.c_str()))
	{
		return vl::string_convert<vl::scalar>(
				XMLNode->first_attribute(attrib.c_str())->value() );
	}
	else
	{ return defaultValue; }
}

bool
DotSceneLoader::getAttribBool( rapidxml::xml_node<>* XMLNode,
		const std::string &attrib, bool defaultValue )
{
	if( !XMLNode->first_attribute(attrib.c_str()) )
	{ return defaultValue; }

	if( std::string(XMLNode->first_attribute(attrib.c_str())->value()) == "true" )
	{ return true; }

	return false;
}

vl::vector
DotSceneLoader::parseVector3(rapidxml::xml_node<>* XMLNode)
{
	return vl::vector(
		vl::string_convert<vl::scalar>(XMLNode->first_attribute("x")->value()),
		vl::string_convert<vl::scalar>(XMLNode->first_attribute("y")->value()),
		vl::string_convert<vl::scalar>(XMLNode->first_attribute("z")->value())
	);
}

vl::quaternion
DotSceneLoader::parseQuaternion(rapidxml::xml_node<>* XMLNode)
{
	//! @todo Fix this crap!

	vl::scalar x, y, z, w;

	if(XMLNode->first_attribute("qx"))
	{
		x = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qx")->value());
		y = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qy")->value());
		z = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qz")->value());
		w = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qw")->value());
	}
	if(XMLNode->first_attribute("qw"))
	{
		w = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qw")->value());
		x = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qx")->value());
		y = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qy")->value());
		z = vl::string_convert<vl::scalar>(XMLNode->first_attribute("qz")->value());
	}
	else if(XMLNode->first_attribute("axisX"))
	{
		// TODO implement
//		vl::vector axis;
//		axis.x = vl::string_convert<vl::scalar>(XMLNode->first_attribute("axisX")->value());
//		axis.y = vl::string_convert<vl::scalar>(XMLNode->first_attribute("axisY")->value());
//		axis.z = vl::string_convert<vl::scalar>(XMLNode->first_attribute("axisZ")->value());
//		vl::scalar angle = vl::string_convert<vl::scalar>(XMLNode->first_attribute("angle")->value());;
//		orientation.FromAngleAxis(Ogre::Angle(angle), axis);
	}
	else if(XMLNode->first_attribute("angleX"))
	{
//		vl::vector axis;
//		axis.x = vl::string_convert<vl::scalar>(XMLNode->first_attribute("angleX")->value());
//		axis.y = vl::string_convert<vl::scalar>(XMLNode->first_attribute("angleY")->value());
//		axis.z = vl::string_convert<vl::scalar>(XMLNode->first_attribute("angleZ")->value());
		//orientation.FromAxes(&axis);
		//orientation.F
	}
	else if(XMLNode->first_attribute("x"))
	{
		x = vl::string_convert<vl::scalar>(XMLNode->first_attribute("x")->value());
		y = vl::string_convert<vl::scalar>(XMLNode->first_attribute("y")->value());
		z = vl::string_convert<vl::scalar>(XMLNode->first_attribute("z")->value());
		w = vl::string_convert<vl::scalar>(XMLNode->first_attribute("w")->value());
	}
	else if(XMLNode->first_attribute("w"))
	{
		w = vl::string_convert<vl::scalar>(XMLNode->first_attribute("w")->value());
		x = vl::string_convert<vl::scalar>(XMLNode->first_attribute("x")->value());
		y = vl::string_convert<vl::scalar>(XMLNode->first_attribute("y")->value());
		z = vl::string_convert<vl::scalar>(XMLNode->first_attribute("z")->value());
	}

	return vl::quaternion(x, y, z, w);
}

vl::colour
DotSceneLoader::parseColour(rapidxml::xml_node<>* XMLNode)
{
	vl::scalar r, g, b, a;
	r = vl::string_convert<vl::scalar>( XMLNode->first_attribute("r")->value() );
	g = vl::string_convert<vl::scalar>( XMLNode->first_attribute("g")->value() );
	b = vl::string_convert<vl::scalar>( XMLNode->first_attribute("b")->value() );
	if(  XMLNode->first_attribute("a") != NULL )
	{ a = vl::string_convert<vl::scalar>( XMLNode->first_attribute("a")->value() ); }
	else
	{ a = 1; }

	return vl::colour(r, g, b, a);
}

std::string
DotSceneLoader::getProperty(const std::string &ndNm,
		const std::string &prop)
{
	for ( unsigned int i = 0 ; i < nodeProperties.size(); i++ )
	{
		if ( nodeProperties[i].nodeName == ndNm && nodeProperties[i].propertyNm == prop )
		{
			return nodeProperties[i].valueName;
		}
	}

	return "";
}

/*
void
DotSceneLoader::processUserDataReference( rapidxml::xml_node<>* XMLNode,
		vl::graph::EntityRefPtr pEntity )
{
	std::string str = XMLNode->first_attribute("id")->value();
	pEntity->setUserAny(Ogre::Any(str));
}
*/