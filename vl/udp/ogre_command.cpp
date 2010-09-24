#include "ogre_command.hpp"

/// Factory method
vl::udp::OgreCommand *
vl::udp::OgreCommand::create( std::string const &cmd_name, Ogre::SceneNode *node )
{
	std::string str(cmd_name);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	if( str == "setposition" )
	{
		return create( CMD_POS, node );
	}
	else if( str == "setquaternion" )
	{
		return create( CMD_ROT_QUAT, node );
	}
	else if( str == "setangleaxis" )
	{
		return create( CMD_ROT_AA, node );
	}
	else if( str == "setangle" )
	{
		return create( CMD_ROT_ANGLE, node );
	}
	else
	{
		return 0;
	}
}

vl::udp::OgreCommand *
vl::udp::OgreCommand::create( CMD_TYPE cmd_type, Ogre::SceneNode *node )
{
	switch( cmd_type )
	{
		case CMD_POS :
			return new SetPosition( node );
			
		case CMD_ROT_QUAT :
			return new SetQuaternion( node );
			
		case CMD_ROT_AA :
			return new SetAngleAxis( node );
			
		case CMD_ROT_ANGLE :
			return new SetAngle( node );

		default :
			return 0;
	}
}

vl::udp::OgreCommand::OgreCommand ( Ogre::SceneNode* node )
	: _node(node)
{
}


vl::udp::SetAngle::SetAngle(Ogre::SceneNode* node)
	: OgreCommand(node)
{
	_data.resize( getSize() );
}

vl::udp::SetAngleAxis::SetAngleAxis(Ogre::SceneNode* node)
	: OgreCommand(node)
{
	_data.resize( getSize() );
}

vl::udp::SetPosition::SetPosition(Ogre::SceneNode* node)
	: OgreCommand(node)
{
	_data.resize( getSize() );
}

vl::udp::SetQuaternion::SetQuaternion(Ogre::SceneNode* node)
	: OgreCommand(node)
{
	_data.resize( getSize() );
}

void
vl::udp::SetPosition::operator() ( void )
{
	if( !_node )
	{ return; }

	Ogre::Vector3 v( _data.at(0), _data.at(1), _data.at(2) );
	_node->setPosition(v);
}

void
vl::udp::SetAngle::operator() ( void )
{
	if( !_node )
	{ return; }

	// TODO implement
}

void
vl::udp::SetQuaternion::operator() ( void )
{
	if( !_node )
	{ return; }

	Ogre::Quaternion q( _data.at(0), _data.at(1), _data.at(2), _data.at(3) );
	_node->setOrientation(q);
}

void
vl::udp::SetAngleAxis::operator() ( void )
{
	if( !_node )
	{ return; }

	Ogre::Radian angle( _data.at(0) );
	Ogre::Vector3 axis( _data.at(1), _data.at(2), _data.at(3) );
	Ogre::Quaternion q( angle, axis );
	_node->setOrientation(q);
}

std::vector<double> &
vl::udp::SetPosition::operator<<( std::vector<double> &vec )
{
	for( size_t i = 0; i < getSize(); ++i )
	{
		_data.at(i) = vec.at(i);
	}
	vec.erase( vec.begin(), vec.begin()+getSize() );
	return vec;
}

std::vector<double> &
vl::udp::SetQuaternion::operator<<( std::vector<double> &vec )
{
	for( size_t i = 0; i < getSize(); ++i )
	{
		_data.at(i) = vec.at(i);
	}
	vec.erase( vec.begin(), vec.begin()+getSize() );
	return vec;
}

std::vector<double> &
vl::udp::SetAngle::operator<<( std::vector<double> &vec )
{
	// We copy only the angle nothing else
	_data.at(0) = vec.at(0);
	vec.erase( vec.begin() );
	return vec;
}

std::vector<double> &
vl::udp::SetAngleAxis::operator<<( std::vector<double> &vec )
{
	for( size_t i = 0; i < getSize(); ++i )
	{
		_data.at(i) = vec.at(i);
	}
	vec.erase( vec.begin(), vec.begin()+getSize() );
	return vec;
}
