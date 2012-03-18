/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file hsf_loader.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 */

#ifndef HYDRA_HSF_LOADER_HPP
#define HYDRA_HSF_LOADER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include <string>

// Necessary for XML parsing
#include "base/rapidxml.hpp"

#include "resource_manager.hpp"

#include "typedefs.hpp"

namespace vl
{

class HYDRA_API HSFLoader
{
public :
	HSFLoader(void);

	virtual ~HSFLoader(void);

	/**	Parse dotscene which is already loaded to scene_data
	 *	Parameters : scene_data is complete xml file loaded into a string
	 *				 attachNode is the parent node for this DotScene scene
	 *				 sPrependNode is a string which is added to the beging of the
	 *				 name of every node
	 */
	void parseScene( std::string const &scene_data,
			vl::GameManagerPtr game_manager,
			std::string const &sPrependNode = std::string() );

	void parseScene( vl::TextResource &scene_data,
			vl::GameManagerPtr game_manager,
			std::string const &sPrependNode = std::string() );

private :
	void _parse( char *xml_data );

	void processScene(rapidxml::xml_node<> *xml_root);

	void processNodes(rapidxml::xml_node<> *xml_node);
	
	void processEnvironment(rapidxml::xml_node<> *xml_node);

	void processFog(rapidxml::xml_node<> *xml_node);

	void processSky(rapidxml::xml_node<> *xml_node);

	/// @brief process node directly attached to Root
	void processNode(rapidxml::xml_node<> *xml_node);

	/// @brief process child nodes of direct childs
	void processChildNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr parent);

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

	vl::GameManagerPtr _game;

	std::string _sPrependNode;

};	// class DotSceneLoader

}	// namespace vl


#endif	// HYDRA_HSF_LOADER_HPP
