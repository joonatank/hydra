/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file: ogre_xml_helpers.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**	Helper functions for parsing XML files
 *	Depends on Ogre Math
 */

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreColourValue.h>

#include <string>

#include "base/rapidxml.hpp"

namespace vl
{

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

}	// namespace vl
