#include "ogre_command.hpp"

#include "base/exceptions.hpp"

/// Factory method
vl::udp::CommandRefPtr
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
		return CommandRefPtr();
	}
}

vl::udp::CommandRefPtr
vl::udp::OgreCommand::create( CMD_TYPE cmd_type, Ogre::SceneNode *node )
{
	switch( cmd_type )
	{
		case CMD_POS :
			return CommandRefPtr( new SetPosition( node ) );
			
		case CMD_ROT_QUAT :
			return CommandRefPtr( new SetQuaternion( node ) );
			
		case CMD_ROT_AA :
			return CommandRefPtr( new SetAngleAxis( node ) );
			
		case CMD_ROT_ANGLE :
			return CommandRefPtr( new SetAngle( node ) );

		default :
			return CommandRefPtr();
	}
}

vl::udp::OgreCommand::OgreCommand ( Ogre::SceneNode* node )
	: _node(node)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
}

void
vl::udp::OgreCommand::check(size_t size, std::vector< double >& vec)
{
	if( vec.size() < size )
	{ BOOST_THROW_EXCEPTION( vl::short_message() ); }
}

void
vl::udp::OgreCommand::remove(size_t size, std::vector< double >& vec)
{
	vec.erase( vec.begin(), vec.begin()+size );
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

void vl::udp::SetPosition::copy(size_t size, std::vector< double >& vec)
{
	for( size_t i = 0; i < size; ++i )
	{
		_pos[i] = vec.at(i);
	}
}

void
vl::udp::SetPosition::print(std::ostream& os) const
{
	os << "SetPosition Command with value = " << _pos;
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

Ogre::Radian
vl::udp::SetOrientation::getAngle( void ) const
{
	Ogre::Radian angle;
	Ogre::Vector3 v;
	_quat.ToAngleAxis(angle, v);
	return angle;
}

void
vl::udp::SetOrientation::setAngle( const Ogre::Radian& angle )
{
	Ogre::Radian a;
	Ogre::Vector3 axis;
	_quat.ToAngleAxis(a, axis);
	_quat.FromAngleAxis(angle, axis);
}

Ogre::Vector3
vl::udp::SetOrientation::getAxis( void ) const
{
	Ogre::Radian a;
	Ogre::Vector3 axis;
	_quat.ToAngleAxis(a, axis);
	return axis;
}

void
vl::udp::SetOrientation::setAxis( const Ogre::Vector3& axis )
{
	Ogre::Radian angle;
	Ogre::Vector3 v;
	_quat.ToAngleAxis(angle, v);
	_quat.FromAngleAxis(angle, axis);
}

void
vl::udp::SetOrientation::getAngleAxis( Ogre::Radian& angle, Ogre::Vector3& axis ) const
{
	_quat.ToAngleAxis(angle, axis);
}

void
vl::udp::SetOrientation::setAngleAxis( const Ogre::Radian& angle, const Ogre::Vector3& axis )
{
	_quat.FromAngleAxis(angle, axis);
}

void
vl::udp::SetOrientation::print( std::ostream &os ) const
{
	os << "SetOrientation Command with value = " << _quat;
}

/// SetQuaternion Command ///
vl::udp::SetQuaternion::SetQuaternion(Ogre::SceneNode* node)
	: SetOrientation(node)
{}

void vl::udp::SetQuaternion::copy(size_t size, std::vector< double >& vec)
{
	for( size_t i = 0; i < size; ++i )
	{
		getQuaternion()[i] = vec.at(i);
	}
}


/// SetAngle Command ///
vl::udp::SetAngle::SetAngle(Ogre::SceneNode* node)
	: SetOrientation(node)
{}

void vl::udp::SetAngle::copy(size_t size, std::vector< double >& vec)
{
	// New angle
	setAngle( Ogre::Radian( vec.at(0) ) );
}

/// SetAngleAxis Command ///
vl::udp::SetAngleAxis::SetAngleAxis(Ogre::SceneNode* node)
	: SetOrientation(node)
{}

void
vl::udp::SetAngleAxis::copy(size_t size, std::vector< double >& vec)
{
	// New Axis
	Ogre::Vector3 axis;
	for( size_t i = 0; i < size-1; ++i )
	{
		axis[i] = vec.at(i+1);
	}

	setAngleAxis( Ogre::Radian( vec.at(0) ), axis );
}
