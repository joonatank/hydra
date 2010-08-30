#include "ogre_command.hpp"

vl::udp::OgreCommand::OgreCommand ( const std::string& typ, Ogre::SceneNode* node ) 
	: _node(node)
{
	std::string str(typ);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	if( str == "setposition" )
	{
		_type = CMD_POS;
		_data.resize(3);
	}
	else if( str == "setquaternion" )
	{
		_type = CMD_ROT_QUAT;
		_data.resize(4);
	}
	else if( str == "setangle" )
	{
		_type = CMD_ROT_AA;
		_data.resize(4);
	}
}


void
vl::udp::OgreCommand::operator() ( void )
{
	if( !_node )
	{ return; }

	if( _type == CMD_POS )
	{
		Ogre::Vector3 v( _data.at(0), _data.at(1), _data.at(2) );
		_node->setPosition(v);
	}
	else if( _type == CMD_ROT_QUAT )
	{
		Ogre::Quaternion q( _data.at(0), _data.at(1), _data.at(2), _data.at(3) );
		_node->setOrientation(q);
	}
	else if( _type == CMD_ROT_AA )
	{
		Ogre::Radian angle( _data.at(0) );
		Ogre::Vector3 axis( _data.at(1), _data.at(2), _data.at(3) );
		Ogre::Quaternion q( angle, axis ); 
		_node->setOrientation(q);
	}
}

uint16_t
vl::udp::OgreCommand::getSize ( void ) const
{
	if( _type == CMD_POS )
	{ return 3; }
	if( _type == CMD_ROT_QUAT )
	{ return 4; }
	if( _type == CMD_ROT_AA )
	{ return 4; }
	
	return 0;
}
