/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Helper functions for parsing XML files
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
