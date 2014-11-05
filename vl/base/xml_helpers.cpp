/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file: base/xml_helpers.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/// Interface
#include "xml_helpers.hpp"
// Necessary for excpetions
#include "base/exceptions.hpp"

Ogre::Real
vl::getAttribReal( rapidxml::xml_node<>* XMLNode,
		const std::string &attrib, Ogre::Real defaultValue )
{
	if(XMLNode->first_attribute(attrib.c_str()))
	{
		return vl::from_string<Ogre::Real>(
				XMLNode->first_attribute(attrib.c_str())->value() );
	}
	else
	{ return defaultValue; }
}

Ogre::Vector3
vl::parseVector3(rapidxml::xml_node<>* XMLNode)
{
	return Ogre::Vector3(
		vl::from_string<Ogre::Real>(XMLNode->first_attribute("x")->value()),
		vl::from_string<Ogre::Real>(XMLNode->first_attribute("y")->value()),
		vl::from_string<Ogre::Real>(XMLNode->first_attribute("z")->value())
	);
}

Ogre::Quaternion
vl::parseQuaternion(rapidxml::xml_node<>* XMLNode)
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
		BOOST_THROW_EXCEPTION( vl::invalid_dotscene() );
	}

	x = vl::from_string<Ogre::Real>( attrX->value() );
	y = vl::from_string<Ogre::Real>( attrY->value() );
	z = vl::from_string<Ogre::Real>( attrZ->value() );
	w = vl::from_string<Ogre::Real>( attrW->value() );

	return Ogre::Quaternion( w, x, y, z );
}

vl::Transform
vl::parseTransform(rapidxml::xml_node<>* xml_node)
{
	rapidxml::xml_node<> *vec = xml_node->first_node("position");
	rapidxml::xml_node<> *quat = xml_node->first_node("quaternion");

	assert(vec && quat);

	Ogre::Vector3 v = parseVector3(vec);
	Ogre::Quaternion q = parseQuaternion(quat);

	return Transform(v, q);
}

Ogre::ColourValue
vl::parseColour(rapidxml::xml_node<>* XMLNode)
{
	Ogre::Real r, g, b, a;
	r = vl::from_string<Ogre::Real>( XMLNode->first_attribute("r")->value() );
	g = vl::from_string<Ogre::Real>( XMLNode->first_attribute("g")->value() );
	b = vl::from_string<Ogre::Real>( XMLNode->first_attribute("b")->value() );
	if(  XMLNode->first_attribute("a") != NULL )
	{ a = vl::from_string<Ogre::Real>( XMLNode->first_attribute("a")->value() ); }
	else
	{ a = 1; }

	return Ogre::ColourValue(r, g, b, a);
}
