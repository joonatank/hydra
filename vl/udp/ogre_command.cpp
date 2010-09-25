#include "ogre_command.hpp"

#include "base/exceptions.hpp"

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
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
}

/// SetPosition Command ///
vl::udp::SetPosition::SetPosition(Ogre::SceneNode* node)
	: OgreCommand(node), _pos( Ogre::Vector3::ZERO )
{}

void
vl::udp::SetPosition::operator() ( void )
{
	_node->setPosition(_pos);
}

std::vector<double> &
vl::udp::SetPosition::operator<<( std::vector<double> &vec )
{
	size_t const size = 3;
	//	TODO add checking if the vector has enough elements
	for( size_t i = 0; i < size; ++i )
	{
		_pos[i] = vec.at(i);
	}
	
	vec.erase( vec.begin(), vec.begin()+size );
	return vec;
}

std::ostream &
vl::udp::SetPosition::operator<<(std::ostream& os) const
{
	os << "SetPosition Command with value = " << _pos;
	return os;
}


/// SetOrientation Command ///
vl::udp::SetOrientation::SetOrientation(Ogre::SceneNode* node)
	: OgreCommand(node), _quat( Ogre::Quaternion::IDENTITY )
{}

void
vl::udp::SetOrientation::operator()(void )
{
	_node->setOrientation(_quat);
}

std::ostream &
vl::udp::SetOrientation::operator<<( std::ostream &os ) const
{
	os << "SetOrientation Command with value = " << _quat;
	return os;
}

/// SetQuaternion Command ///
vl::udp::SetQuaternion::SetQuaternion(Ogre::SceneNode* node)
	: SetOrientation(node)
{}

std::vector<double> &
vl::udp::SetQuaternion::operator<<( std::vector<double> &vec )
{
	size_t const size = 4;
	for( size_t i = 0; i < size; ++i )
	{
		getQuaternion()[i] = vec.at(i);
	}
	vec.erase( vec.begin(), vec.begin()+size );
	return vec;
}


/// SetAngle Command ///
vl::udp::SetAngle::SetAngle(Ogre::SceneNode* node)
	: SetOrientation(node)
{}

std::vector<double> &
vl::udp::SetAngle::operator<<( std::vector<double> &vec )
{
	Ogre::Radian angle;
	Ogre::Vector3 axis;
	_quat.ToAngleAxis(angle, axis);

	//	TODO add checking if the vector has enough elements
	
	// New angle
	angle = vec.at(0);
	vec.erase( vec.begin() );

	_quat.FromAngleAxis(angle, axis);

	return vec;
}

/// SetAngleAxis Command ///
vl::udp::SetAngleAxis::SetAngleAxis(Ogre::SceneNode* node)
	: SetOrientation(node)
{}

std::vector<double> &
vl::udp::SetAngleAxis::operator<<( std::vector<double> &vec )
{
	Ogre::Radian angle;
	Ogre::Vector3 axis;

	size_t const size = 3;
	//	TODO add checking if the vector has enough elements
	// New angle
	angle = vec.at(0);
	for( size_t i = 0; i < size; ++i )
	{
		axis[i] = vec.at(i+1);
	}
	
	vec.erase( vec.begin(), vec.begin()+size+1 );

	_quat.FromAngleAxis(angle, axis);

	return vec;
}
