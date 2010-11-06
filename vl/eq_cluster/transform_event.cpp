
#include "transform_event.hpp"

std::string const eqOgre::TransformOperation::TYPENAME = "TransformOperation";

std::string const eqOgre::SetTransformationOperation::TYPENAME = "SetTransformationOperation";

std::string const eqOgre::MoveOperation::TYPENAME = "MoveOperation";

eqOgre::TransformationEvent::TransformationEvent(eqOgre::SceneNode* node)
	: _node(node), _last_time( ::clock() ), _speed(1),
		_angular_speed( Ogre::Degree(60) ),
		_operation( new MoveOperation(node) )
//		_move_keys(), _rot_keys(),
//		_move_dir( Ogre::Vector3::ZERO ),
//		_rotation_axises( Ogre::Vector3::ZERO )
{
//	if( !_node )
//	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_trans_triggers.resize(3);;
	_rot_triggers.resize(3);;
}

std::string const eqOgre::TransformationEvent::TYPENAME = "TransformationEvent";

eqOgre::TransformationEvent::TransformationEvent(const eqOgre::TransformationEvent& a)
{
	*this = a;
}


eqOgre::TransformationEvent &
eqOgre::TransformationEvent::operator=(const eqOgre::TransformationEvent& a)
{
	/*
	_node = a._node;
	_last_time = a._last_time;
	_speed = a._speed;
	_angular_speed = a._angular_speed;
	_move_keys = a._move_keys;
	_rot_keys = a._rot_keys;

	// Temporary variables need to be set to default values (zero)
	_move_dir =  Ogre::Vector3::ZERO;
	_rotation_axises = Ogre::Vector3::ZERO;
	*/

	return *this;
}

void
eqOgre::TransformationEvent::operator()(void )
{
	clock_t time = ::clock();
	// Secs since last frame
	double t = ((double)( time - _last_time ))/CLOCKS_PER_SEC;

	_operation->execute( t );

	_last_time = time;
}

bool
eqOgre::TransformationEvent::processTrigger(eqOgre::Trigger* trig)
{
	if( _trans_triggers.at(0)._trig1 == trig )
	{
		_operation->addMove( Ogre::Vector3::UNIT_X );
		return true;
	}
	else if( _trans_triggers.at(0)._trig2 == trig )
	{
		_operation->addMove( -Ogre::Vector3::UNIT_X );
		return true;
	}
	else if( _trans_triggers.at(1)._trig1 == trig )
	{
		_operation->addMove( Ogre::Vector3::UNIT_Y );
		return true;
	}
	else if( _trans_triggers.at(1)._trig1 == trig )
	{
		_operation->addMove( -Ogre::Vector3::UNIT_Y );
		return true;
	}
	else if( _trans_triggers.at(2)._trig1 == trig )
	{
		_operation->addMove( Ogre::Vector3::UNIT_Z );
		return true;
	}
	else if( _trans_triggers.at(2)._trig1 == trig )
	{
		_operation->addMove( -Ogre::Vector3::UNIT_Z );
		return true;
	}

	else if( _rot_triggers.at(0)._trig1 == trig )
	{
		_operation->addRotation( Ogre::Vector3::UNIT_X );
		return true;
	}
	else if( _rot_triggers.at(0)._trig2 == trig )
	{
		_operation->addRotation( -Ogre::Vector3::UNIT_X );
		return true;
	}
	else if( _rot_triggers.at(1)._trig1 == trig )
	{
		_operation->addRotation( Ogre::Vector3::UNIT_Y );
		return true;
	}
	else if( _rot_triggers.at(1)._trig2 == trig )
	{
		_operation->addRotation( -Ogre::Vector3::UNIT_Y );
		return true;
	}
	else if( _rot_triggers.at(2)._trig1 == trig )
	{
		_operation->addRotation( Ogre::Vector3::UNIT_Z );
		return true;
	}
	else if( _rot_triggers.at(2)._trig2 == trig )
	{
		_operation->addRotation( -Ogre::Vector3::UNIT_Z );
		return true;
	}

	else if( FrameTrigger() == *trig )
	{
		(*this)();
		return true;
	}

	return false;
}

/*
bool eqOgre::TransformationEvent::keyPressed(OIS::KeyCode key)
{
	bool retval = false;
	Ogre::Vector3 vec = _move_keys.findKey(key);
	if( vec != Ogre::Vector3::ZERO )
	{
		_move_dir += vec;
		retval = true;
	}

	vec = _rot_keys.findKey(key);
	if( vec != Ogre::Vector3::ZERO )
	{
		_rotation_axises += vec;
		retval = true;
	}

	return retval;
}

bool eqOgre::TransformationEvent::keyReleased(OIS::KeyCode key)
{
	bool retval = false;
	Ogre::Vector3 vec = _move_keys.findKey(key);
	if( vec != Ogre::Vector3::ZERO )
	{
		_move_dir -= vec;
		retval = true;
	}

	vec = _rot_keys.findKey(key);
	if( vec != Ogre::Vector3::ZERO )
	{
		_rotation_axises -= vec;
		retval = true;
	}

	return retval;
}
*/
