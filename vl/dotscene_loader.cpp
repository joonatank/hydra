/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include "dotscene_loader.hpp"

#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"

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

	rapidxml::xml_node<>* XMLRoot;

	XMLDoc.parse<0>( xml_data );

	// Grab the scene node
	XMLRoot = XMLDoc.first_node("scene");

	// Process the scene
	processScene(XMLRoot);
}


/// ------- DotSceneLoader Private -------------
void
vl::DotSceneLoader::processScene(rapidxml::xml_node<> *xml_root)
{
	rapidxml::xml_node<>* pElement;

	// Process nodes (?)
	pElement = xml_root->first_node("nodes");
	if( pElement )
	{ processNodes(pElement); }
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
