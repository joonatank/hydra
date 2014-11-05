/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file dotscene_loader.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	DotScene file loader for distributed objects. Creates the object in config.
 *	So that they can be retrieved with getSceneNode later.
 *
 *	Does NOT support any other types than SceneNodes for now.
 *	Also does NOT support anything else from SceneNode than name and transformation.
 */

#ifndef HYDRA_DOTSCENE_LOADER_HPP
#define HYDRA_DOTSCENE_LOADER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include <string>

// Necessary for XML parsing
#include "base/rapidxml.hpp"

#include "resource_manager.hpp"

#include "typedefs.hpp"

namespace vl
{

class HYDRA_API DotSceneLoader
{
public :
	DotSceneLoader(bool use_new_mesh_manager = false);

	virtual ~DotSceneLoader();

	/**	Parse dotscene which is already loaded to scene_data
	 *	Parameters : scene_data is complete xml file loaded into a string
	 *				 attachNode is the parent node for this DotScene scene
	 *				 sPrependNode is a string which is added to the beging of the
	 *				 name of every node
	 */
	void parseDotScene( std::string const &scene_data,
			vl::SceneManagerPtr scene_manager,
			vl::physics::WorldRefPtr physics_world = vl::physics::WorldRefPtr(),
			vl::SceneNodePtr attachNode = 0,
			std::string const &sPrependNode = std::string() );

	void parseDotScene( vl::TextResource &scene_data,
			vl::SceneManagerPtr scene_manager,
			vl::physics::WorldRefPtr physics_world = vl::physics::WorldRefPtr(),
			vl::SceneNodePtr attachNode = 0,
			std::string const &sPrependNode = std::string() );

private :
	void _parse( char *xml_data );

	void processScene(rapidxml::xml_node<> *xml_root);

	void parseSceneHeader(rapidxml::xml_node<> *xml_root);

	void processNodes(rapidxml::xml_node<> *xml_node);
	
	/**	Not supported elements
	 *	skyBox
	 *	skyPlane
	 *	clipping
	 *	colourBackground
	 */
	void processEnvironment(rapidxml::xml_node<> *xml_node);

	void processFog(rapidxml::xml_node<> *xml_node);
	void processSkyDome(rapidxml::xml_node<> *xml_node);

	/**	Not supported elements
	 *	lookTarget
	 *	trackTarget
	 *	particleSystem
	 *	billboardSet
	 *	plane
	 */
	void processNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent);

	void processEntity(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent);

	void processLight(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent);
	
	/** Not supported elements
	 *	normal
	 *	lookTarget
	 *	trackTarget
	 *	userDataReference
	 */
	void processCamera(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent);

	void processLightRange(rapidxml::xml_node<> *xml_node, vl::LightPtr light);

	void processLightAttenuation(rapidxml::xml_node<> *xml_node, vl::LightPtr light);

	vl::SceneManagerPtr _scene;
	vl::physics::WorldRefPtr _physics_world;

	vl::SceneNodePtr _attach_node;

	std::string _sPrependNode;

	bool _use_new_mesh_manager;

	std::string _file_name;

};	// class DotSceneLoader

}	// namespace vl


#endif	// HYDRA_DOTSCENE_LOADER_HPP
