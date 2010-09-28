#include "dotscene_loader.hpp"

#include <sstream>

#include "base/string_utils.hpp"
#include "base/exceptions.hpp"

/* For now we are using Ogre::LogManager and Ogre::ResourceGroupManager
 * Because we don't have our own logging or resource system in place.
 * TODO should be removed as soon as possible.
 */

#include <OgreLogManager.h>
#include <OgreException.h>
#include <OgreResourceManager.h>
#include <OgreEntity.h>

DotSceneLoader::DotSceneLoader()
	: _xml_data(0)
{
}

DotSceneLoader::~DotSceneLoader()
{
	delete [] _xml_data;
}
void
DotSceneLoader::parseDotScene(
		std::string const &sceneName,
		std::string const &groupName,
		Ogre::SceneManager *sceneMgr,
		Ogre::SceneNode *attachNode,
		std::string const &sPrependNode )
{
	// set up shared object values
	_sGroupName = groupName;

	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton()
		.openResource( sceneName, groupName );
		
	parseDotScene( stream->getAsString(), sceneMgr, attachNode, sPrependNode );
}

void
DotSceneLoader::parseDotScene(
		std::string const &scene_data,
		Ogre::SceneManager *sceneMgr,
		Ogre::SceneNode *attachNode,
		std::string const &sPrependNode )
{
	// set up shared object values
	_scene_mgr = sceneMgr;
	_attach_node = attachNode;
	_sPrependNode = sPrependNode;
	staticObjects.clear();
	dynamicObjects.clear();

	rapidxml::xml_document<> XMLDoc;    // character type defaults to char

	rapidxml::xml_node<>* XMLRoot;

	delete [] _xml_data;
	_xml_data = new char[scene_data.length()+1];
	::strcpy( _xml_data, scene_data.c_str() );
	
//	std::ifstream fs( scenePath.c_str(), std::ios::binary );
//	_xml_data.readStream( fs );
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

	if( !_attach_node )
	{ _attach_node = _scene_mgr->getRootSceneNode(); }

	// Process the scene	
	processScene(XMLRoot);
}

void
DotSceneLoader::processScene(rapidxml::xml_node<>* XMLRoot)
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
	try {
		pElement = XMLRoot->first_node("environment");
		if(pElement)
		{ processEnvironment(pElement); }
	}
	catch( Ogre::Exception const &e )
	{
		std::string message = "Ogre Exception : " + std::string(e.what());
		Ogre::LogManager::getSingleton().logMessage(message);
	}

	// Process nodes (?)
	pElement = XMLRoot->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }

	// Process externals (?)
	pElement = XMLRoot->first_node("externals");
	if(pElement)
	{ processExternals(pElement); }

	// Process userDataReference (?)
	/*
	pElement = XMLRoot->first_node("userDataReference");
	if(pElement)
	{ processUserDataReference(pElement); }
	*/
	
	// Process octree (?)
	/*
	pElement = XMLRoot->first_node("octree");
	if(pElement)
	{ processOctree(pElement); }
	*/
	
	// Process light (?)
	pElement = XMLRoot->first_node("light");
	if(pElement)
	{ processLight(pElement); }

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
	pElement = XMLNode->first_node("rotation");
	if( pElement )
	{ 
		_attach_node->setOrientation(parseQuaternion(pElement)); 
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
	{ _scene_mgr->setAmbientLight( parseColour(pElement) ); }

	// Process colourBackground (?)
	//! @todo Set the background colour of all viewports (RenderWindow has to be provided then)
	pElement = XMLNode->first_node("colourBackground");
	if( pElement )
		;

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if( pElement )
	{ processUserDataReference(pElement); }
}

/// Not implemented
void
DotSceneLoader::processTerrain(rapidxml::xml_node<>* XMLNode)
{
	/*	Terrain not yet Implemented
	Ogre::Real worldSize = getAttribReal(XMLNode, "worldSize");
	int mapSize = std::stringConverter::parseInt(XMLNode->first_attribute("mapSize")->value());
	//	Not used
	int rows = std::stringConverter::parseInt(XMLNode->first_attribute("rows")->value());
	int columns = std::stringConverter::parseInt(XMLNode->first_attribute("columns")->value());
	bool colourmapEnabled = getAttribBool(XMLNode, "colourmapEnabled");
	int colourMapTextureSize = std::stringConverter::
			parseInt(XMLNode->first_attribute("colourMapTextureSize")->value());

	Ogre::Vector33 lightdir(0, -0.3, 0.75);
	lightdir.normalise();
	vl::graph::LightRefPtr* l = scene_mgr->createLight("tstLight");
	l->setType(vl::graph::LightRefPtr::LT_DIRECTIONAL);
	l->setDirection(lightdir);
	l->setDiffuseColour(vl::ColourValue(1.0, 1.0, 1.0));
	l->setSpecularColour(vl::ColourValue(0.4, 0.4, 0.4));
	scene_mgr->setAmbientLight(vl::ColourValue(0.6, 0.6, 0.6));

	mTerrainGlobalOptions->setMaxPixelError(1);
	mTerrainGlobalOptions->setCompositeMapDistance(2000);
	mTerrainGlobalOptions->setLightMapDirection(lightdir);
	mTerrainGlobalOptions->setCompositeMapAmbient( scene_mgr->getAmbientLight() );
	mTerrainGlobalOptions->setCompositeMapDiffuse(l->getDiffuseColour());

	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup( scene_mgr,
			Ogre::Terrain::ALIGN_X_Z, mapSize, worldSize);
	mTerrainGroup->setOrigin(Ogre::Vector33::ZERO);

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

/// Not implemented
void
DotSceneLoader::processTerrainPage(rapidxml::xml_node<>* XMLNode)
{
	/*
	std::string name = getAttrib(XMLNode, "name");
	int pageX = std::stringConverter::parseInt(XMLNode->first_attribute("pageX")->value());
	int pageY = std::stringConverter::parseInt(XMLNode->first_attribute("pageY")->value());
	Ogre::Real worldSize = getAttribReal(XMLNode, "worldSize");

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
                    imp.layerList[count].worldSize = (Ogre::Real)worldSize;
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

/// Not implemented
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

/// Not implemented
void
DotSceneLoader::processUserDataReference(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
{
	//! @todo Implement this
}

/// Not implemented
void
DotSceneLoader::processOctree(rapidxml::xml_node<>* XMLNode)
{
	//! @todo Implement this
}

void
DotSceneLoader::processLight(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
{
	// If no parent is provided we use Root node
	if( !parent )
	{ parent = _attach_node; }

	// Process attributes
	std::string name = getAttrib(XMLNode, "name");
	std::string id = getAttrib(XMLNode, "id");

	// Create the light
	Ogre::Light *light = _scene_mgr->createLight(name);

	parent->attachObject(light);
	// Create light node
//	std::string node_name = name + "Node";
//	Ogre::SceneNode *light_node
//		= parent->createChild( node_name );
//	light_node->attachObject(light);

	std::string sValue = getAttrib(XMLNode, "type");
	if(sValue == "point")
	{ light->setType( Ogre::Light::LT_POINT ); }
	else if(sValue == "directional")
	{ light->setType( Ogre::Light::LT_DIRECTIONAL ); }
	else if(sValue == "spot" )// || sValue == "spotLight" )
	{ light->setType( Ogre::Light::LT_SPOTLIGHT ); }
	else if(sValue == "radPoint")
	{ light->setType( Ogre::Light::LT_POINT ); }

	light->setVisible(getAttribBool(XMLNode, "visible", true));
	light->setCastShadows(getAttribBool(XMLNode, "castShadows", true));

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if(pElement)
	{ light->setPosition(parseVector3(pElement)); }

	// Process normal (?)
	pElement = XMLNode->first_node("normal");
	if(pElement)
	{ light->setDirection(parseVector3(pElement)); }

	pElement = XMLNode->first_node("directionVector");
	if(pElement)
	{ light->setDirection(parseVector3(pElement)); }

	// Process colourDiffuse (?)
	pElement = XMLNode->first_node("colourDiffuse");
	if(pElement)
	{ light->setDiffuseColour(parseColour(pElement)); }

	// Process colourSpecular (?)
	pElement = XMLNode->first_node("colourSpecular");
	if(pElement)
	{ light->setSpecularColour(parseColour(pElement)); }

	if(sValue == "spot" )//|| sValue == "spotLight" )
	{
		// Process lightRange (?)
		pElement = XMLNode->first_node("lightRange");
		if(pElement)
		{ processLightRange(pElement, light); }
	}
	if( sValue != "direction" )
	{
		// Process lightAttenuation (?)
		pElement = XMLNode->first_node("lightAttenuation");
		if(pElement)
		{ processLightAttenuation(pElement, light); }
	}

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if(pElement)
		;//processUserDataReference(pElement, light);
}

void
DotSceneLoader::processCamera(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
{
	// If no parent is provided we use Root node
	if( !parent )
	{ parent = _attach_node; }

	// Process attributes
	std::string name = getAttrib(XMLNode, "name");
	std::string id = getAttrib(XMLNode, "id");

	std::string node_name = name + std::string("Node");
	// Create the camera
	Ogre::Camera *camera = _scene_mgr->createCamera( name );
	parent->attachObject( camera );
//	Ogre::SceneNode *cam_node
//		= parent->createChildSceneNode( node_name );
//	cam_node->attachObject( camera );

	rapidxml::xml_node<>* pElement;

	// Process clipping (?)
	pElement = XMLNode->first_node("clipping");
	if(pElement)
	{
		Ogre::Real nearDist = getAttribReal(pElement, "near");
		if( nearDist > 0 )
		{ camera->setNearClipDistance(nearDist); }

		Ogre::Real farDist =  getAttribReal(pElement, "far");
		if( farDist > 0 && farDist > nearDist )
		{ camera->setFarClipDistance(farDist); }
	}

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if(pElement)
	{ camera->setPosition( parseVector3(pElement) ); }

	// Process rotation (?)
	pElement = XMLNode->first_node("rotation");
	if(pElement)
	{ camera->setOrientation( parseQuaternion(pElement) ); }

	pElement = XMLNode->first_node("quaternion");
	if(pElement)
	{ camera->setOrientation( parseQuaternion(pElement) ); }
/*
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
*/
	// construct a scenenode is no parent
	/*
	if( !parent )
	{
		vl::graph::SceneNodeRefPtr* node = _attach_node->createChild(name);
		node->setPosition(camera->getPosition());
		node->setOrientation(camera->getOrientation());
		node->scale(1,1,1);
	}
	*/
}

void
DotSceneLoader::processNode(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
{
	// If no parent is provided we use Root node
	if( !parent )
	{ parent = _attach_node; }

	// Construct the node's name
	std::string name = _sPrependNode + getAttrib(XMLNode, "name");

	// Create the scene node
	Ogre::SceneNode *node = parent->createChildSceneNode(name);

	// Process other attributes
//	std::string id = getAttrib(XMLNode, "id");
	// Not used
//	bool isTarget = getAttribBool(XMLNode, "isTarget");

	rapidxml::xml_node<>* pElement;

	// Process position (?)
	pElement = XMLNode->first_node("position");
	if( pElement )
	{ node->setPosition(parseVector3(pElement)); }
	
	// Process rotation (?)
	pElement = XMLNode->first_node("rotation");
	if( pElement )
	{ node->setOrientation(parseQuaternion(pElement)); }

	pElement = XMLNode->first_node("quaternion");
	if( pElement )
	{ node->setOrientation(parseQuaternion(pElement)); }

	// Process scale (?)
	pElement = XMLNode->first_node("scale");
	if(pElement)
	{ node->setScale(parseVector3(pElement)); }

	/*	Process lookTarget (?)
	 *	TODO not supported yet
	pElement = XMLNode->first_node("lookTarget");
	if(pElement)
	{ processLookTarget(pElement, node); }
	*/

	/*	Process trackTarget (?)
	 *	TODO not supported yet
	pElement = XMLNode->first_node("trackTarget");
	if(pElement)
	{ processTrackTarget(pElement, node); }
	*/

	/*	Process node (*) */
	pElement = XMLNode->first_node("node");
	while(pElement)
	{
		processNode(pElement, node);
		pElement = pElement->next_sibling("node");
	}

	/*	Process entity (*) */
	pElement = XMLNode->first_node("entity");
	while(pElement)
	{
		processEntity(pElement, node);
		pElement = pElement->next_sibling("entity");
	}
	
	/*	Process light (*) */
	pElement = XMLNode->first_node("light");
	while(pElement)
	{
		processLight(pElement, node);
		pElement = pElement->next_sibling("light");
	}

	/*	Process camera (*) */
	pElement = XMLNode->first_node("camera");
	while(pElement)
	{
		processCamera(pElement, node);
		pElement = pElement->next_sibling("camera");
	}

	/*	Process particleSystem (*)
	 *	TODO not supported yet
	pElement = XMLNode->first_node("particleSystem");
	while(pElement)
	{
		processParticleSystem(pElement, node);
		pElement = pElement->next_sibling("particleSystem");
	}
	*/

	/*	Process billboardSet (*)
	 *	TODO not supported yet
	pElement = XMLNode->first_node("billboardSet");
	while( pElement )
	{
		processBillboardSet(pElement, node);
		pElement = pElement->next_sibling("billboardSet");
	}
	*/

	/*	Process plane (*)
	 *	TODO not supported yet
	pElement = XMLNode->first_node("plane");
	while( pElement )
	{
		processPlane(pElement, node);
		pElement = pElement->next_sibling("plane");
	}
	*/
	
	/*	Process userDataReference 
	 *	TODO not supported yet
	pElement = XMLNode->first_node("userDataReference");
	if( pElement )
	{ processUserDataReference(pElement, node); }
	*/
}

/// Not implemented
void
DotSceneLoader::processLookTarget(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
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
	Ogre::Vector3 position;
	pElement = XMLNode->first_node("position");
	if(pElement)
		position = parseVector3(pElement);

	// Process localDirection (?)
	Ogre::Vector3 localDirection = Ogre::Vector3::NEGATIVE_UNIT_Z;
	pElement = XMLNode->first_node("localDirection");
	if(pElement)
		localDirection = parseVector3(pElement);

	// Setup the look target
	try
	{
		if( !nodeName.empty() )
		{
			Ogre::SceneNode *pLookNode = sceneMgr->getSceneNode(nodeName);
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

/// Not implemented
void
DotSceneLoader::processTrackTarget(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *parent)
{
	/*	TODO this needs SceneNode::autoTracking
	 *
	// Process attributes
	std::string nodeName = getAttrib(XMLNode, "nodeName");

	rapidxml::xml_node<>* pElement;

	// Process localDirection (?)
	Ogre::Vector3 localDirection = Ogre::Vector3::NEGATIVE_UNIT_Z;
	pElement = XMLNode->first_node("localDirection");
	if( pElement )
	{ localDirection = parseVector3(pElement); }

	// Process offset (?)
	Ogre::Vector3 offset = Ogre::Vector3::ZERO;
	pElement = XMLNode->first_node("offset");
	if( pElement )
	{ offset = parseVector3(pElement); }

	// Setup the track target
	try
	{
		Ogre::SceneNode **pTrackNode = _scene_mgr->getSceneNode(nodeName);
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
		Ogre::SceneNode *parent)
{
	// TODO this needs SceneNode::attachObject,
	// Entity::setCastShadows, Entity::setMaterialName

	// If no parent is provided we use Root node
	if( !parent )
	{ parent = _attach_node; }

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
	Ogre::Entity *entity;
	try
	{
		//Ogre::MeshManager::getSingleton().load(meshFile, _sGroupName);
		entity = _scene_mgr->createEntity(name, meshFile);
		entity->setCastShadows(castShadows);
		parent->attachObject(entity);
		
		if( !materialFile.empty() )
		{ entity->setMaterialName(materialFile); }
	}
	catch(Ogre::Exception &/*e*/)
	{
		std::string msg("[DotSceneLoader] Error loading an entity!");
		std::cerr << msg << std::endl;
		//	TODO add logging
		//Ogre::LogManager::getSingleton().logMessage(msg);
	}

	// Process userDataReference (?)
	pElement = XMLNode->first_node("userDataReference");
	if( pElement )
	{ processUserDataReference(pElement, entity); }
}

/// Not implemented
void
DotSceneLoader::processParticleSystem(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
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
			= scene_mgr->createParticleSystem(name, file);
		parent->attachObject(pParticles);
	}
	catch(Ogre::Exception & )
	{
		std::string msg("[DotSceneLoader] Error creating a particle system!");
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
	*/
}

/// Not implemented
void
DotSceneLoader::processBillboardSet(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
{
	//! @todo Implement this
}

/// Not implemented
void
DotSceneLoader::processPlane(rapidxml::xml_node<>* XMLNode,
		Ogre::SceneNode *parent)
{
/*	TODO implement plane
    std::string name = getAttrib(XMLNode, "name");
    Ogre::Real distance = getAttribReal(XMLNode, "distance");
    Ogre::Real width = getAttribReal(XMLNode, "width");
    Ogre::Real height = getAttribReal(XMLNode, "height");
    int xSegments = std::stringConverter::parseInt(getAttrib(XMLNode, "xSegments"));
    int ySegments = std::stringConverter::parseInt(getAttrib(XMLNode, "ySegments"));
    int numTexCoordSets = std::stringConverter::parseInt(getAttrib(XMLNode, "numTexCoordSets"));
    Ogre::Real uTile = getAttribReal(XMLNode, "uTile");
    Ogre::Real vTile = getAttribReal(XMLNode, "vTile");
    std::string material = getAttrib(XMLNode, "material");
    bool hasNormals = getAttribBool(XMLNode, "hasNormals");
    Ogre::Vector3 normal = parseVector3(XMLNode->first_node("normal"));
    Ogre::Vector3 up = parseVector3(XMLNode->first_node("upVector"));

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
	// Process attributes
	Ogre::Real expDensity = getAttribReal(XMLNode, "density", 0.001);
	Ogre::Real linearStart = getAttribReal(XMLNode, "start", 0.0);
	Ogre::Real linearEnd = getAttribReal(XMLNode, "end", 1.0);

	Ogre::FogMode mode = Ogre::FOG_NONE;
	std::string sMode = getAttrib(XMLNode, "mode");
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
	{ colourDiffuse = parseColour(pElement); }

	// Setup the fog
	_scene_mgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
}

/// Not implemented
void
DotSceneLoader::processSkyBox(rapidxml::xml_node<>* XMLNode)
{
	// Process attributes
	// material attribute is required, all others are optional and have defaults
	std::string material = XMLNode->first_attribute("material")->value();
	Ogre::Real distance = getAttribReal(XMLNode, "distance", 5000);
	bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
	pElement = XMLNode->first_node("rotation");
	if( pElement )
	{ rotation = parseQuaternion(pElement); }

	// Setup the sky box
	_scene_mgr->setSkyBox(true, material, distance, drawFirst, rotation, _sGroupName);
}

/// Not implemented
void
DotSceneLoader::processSkyDome(rapidxml::xml_node<>* XMLNode)
{
	// Process attributes
	// material attribute is required, all others are optional and have defaults
	std::string material = XMLNode->first_attribute("material")->value();
	Ogre::Real curvature = getAttribReal(XMLNode, "curvature", 10);
	Ogre::Real tiling = getAttribReal(XMLNode, "tiling", 8);
	Ogre::Real distance = getAttribReal(XMLNode, "distance", 4000);
	bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

	rapidxml::xml_node<>* pElement;

	// Process rotation (?)
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
	pElement = XMLNode->first_node("rotation");
	if(pElement)
	{ rotation = parseQuaternion(pElement); }

	// Setup the sky dome
	_scene_mgr->setSkyDome( true, material, curvature, tiling,
			distance, drawFirst, rotation, 16, 16, -1, _sGroupName );

	std::string message = "Skydome Created with material " + material;
	Ogre::LogManager::getSingleton().logMessage( message );
}

/// Not implemented
void
DotSceneLoader::processSkyPlane(rapidxml::xml_node<>* XMLNode)
{
//	TODO implement SkyPlane
	// Process attributes
	std::string material = getAttrib(XMLNode, "material");
	Ogre::Real planeX = getAttribReal(XMLNode, "planeX", 0);
	Ogre::Real planeY = getAttribReal(XMLNode, "planeY", -1);
	Ogre::Real planeZ = getAttribReal(XMLNode, "planeX", 0);
	Ogre::Real planeD = getAttribReal(XMLNode, "planeD", 5000);
	Ogre::Real scale = getAttribReal(XMLNode, "scale", 1000);
	Ogre::Real bow = getAttribReal(XMLNode, "bow", 0);
	Ogre::Real tiling = getAttribReal(XMLNode, "tiling", 10);
	bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

	// Setup the sky plane
	Ogre::Plane plane;
	plane.normal = Ogre::Vector3(planeX, planeY, planeZ);
	plane.d = planeD;
	_scene_mgr->setSkyPlane( true, plane, material, scale, tiling,
			drawFirst, bow, 1, 1, _sGroupName );
}

/// Not implemented
void
DotSceneLoader::processClipping(rapidxml::xml_node<>* /*XMLNode */)
{
	//! @todo Implement this

	// Process attributes
//	Ogre::Real fNear = getAttribReal(XMLNode, "near", 0);
//	Ogre::Real fFar = getAttribReal(XMLNode, "far", 1);
}

void
DotSceneLoader::processLightRange(rapidxml::xml_node<>* XMLNode, Ogre::Light *light)
{
	// Process attributes
	Ogre::Real inner = getAttribReal(XMLNode, "inner");
	Ogre::Real outer = getAttribReal(XMLNode, "outer");
	Ogre::Real falloff = getAttribReal(XMLNode, "falloff", 1.0);

	// Setup the light range
	light->setSpotlightRange(Ogre::Angle(inner), Ogre::Angle(outer), falloff);
}

void
DotSceneLoader::processLightAttenuation(rapidxml::xml_node<>* XMLNode,
		Ogre::Light *light)
{
	// Process attributes
	Ogre::Real range = getAttribReal(XMLNode, "range");
	Ogre::Real constant = getAttribReal(XMLNode, "constant");
	Ogre::Real linear = getAttribReal(XMLNode, "linear");
	Ogre::Real quadratic = getAttribReal(XMLNode, "quadratic");

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

Ogre::Real
DotSceneLoader::getAttribReal( rapidxml::xml_node<>* XMLNode,
		const std::string &attrib, Ogre::Real defaultValue )
{
	if(XMLNode->first_attribute(attrib.c_str()))
	{
		return vl::string_convert<Ogre::Real>(
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

Ogre::Vector3
DotSceneLoader::parseVector3(rapidxml::xml_node<>* XMLNode)
{
	return Ogre::Vector3(
		vl::string_convert<Ogre::Real>(XMLNode->first_attribute("x")->value()),
		vl::string_convert<Ogre::Real>(XMLNode->first_attribute("y")->value()),
		vl::string_convert<Ogre::Real>(XMLNode->first_attribute("z")->value())
	);
}

Ogre::Quaternion
DotSceneLoader::parseQuaternion(rapidxml::xml_node<>* XMLNode)
{
	Ogre::Real x, y, z, w;

	// TODO add axisX, axisY, axisZ
	// TODO add angleX, angleY, angleZ

	// Default attribute names
	rapidxml::xml_attribute<> *attrX = XMLNode->first_attribute("qx");
	rapidxml::xml_attribute<> *attrY = XMLNode->first_attribute("qy");
	rapidxml::xml_attribute<> *attrZ = XMLNode->first_attribute("qz");
	rapidxml::xml_attribute<> *attrW = XMLNode->first_attribute("qw");

	// Alternative attribute names
	if( !attrX )
	{ attrX = XMLNode->first_attribute( "x" ); }
	if( !attrY )
	{ attrY = XMLNode->first_attribute( "y" ); }
	if( !attrZ )
	{ attrZ = XMLNode->first_attribute( "z" ); }
	if( !attrW )
	{ attrW = XMLNode->first_attribute( "w" ); }

	if( !attrX || !attrY || !attrZ || !attrW )
	{
		// TODO add description
		BOOST_THROW_EXCEPTION( vl::invalid_dotscene() );
	}

	x = vl::string_convert<Ogre::Real>( attrX->value() );
	y = vl::string_convert<Ogre::Real>( attrY->value() );
	z = vl::string_convert<Ogre::Real>( attrZ->value() );
	w = vl::string_convert<Ogre::Real>( attrW->value() );
	
	return Ogre::Quaternion( w, x, y, z );
}

Ogre::ColourValue
DotSceneLoader::parseColour(rapidxml::xml_node<>* XMLNode)
{
	Ogre::Real r, g, b, a;
	r = vl::string_convert<Ogre::Real>( XMLNode->first_attribute("r")->value() );
	g = vl::string_convert<Ogre::Real>( XMLNode->first_attribute("g")->value() );
	b = vl::string_convert<Ogre::Real>( XMLNode->first_attribute("b")->value() );
	if(  XMLNode->first_attribute("a") != NULL )
	{ a = vl::string_convert<Ogre::Real>( XMLNode->first_attribute("a")->value() ); }
	else
	{ a = 1; }

	return Ogre::ColourValue(r, g, b, a);
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

/// Not implemented
void
DotSceneLoader::processUserDataReference( rapidxml::xml_node<>* XMLNode,
		Ogre::Entity *pEntity )
{
/*
	std::string str = XMLNode->first_attribute("id")->value();
	pEntity->setUserAny(Ogre::Any(str));
*/
}
