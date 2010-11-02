
#include "transform_event.hpp"

eqOgre::TransformationEvent::TransformationEvent(eqOgre::SceneNode* node)
	: _node(node), _last_time( ::clock() ), _speed(1),
		_angular_speed( Ogre::Degree(60) ),
		_move_keys(), _rot_keys(),
		_move_dir( Ogre::Vector3::ZERO ),
		_rotation_axises( Ogre::Vector3::ZERO )
{}

eqOgre::TransformationEvent::TransformationEvent(const eqOgre::TransformationEvent& a)
{
	*this = a;
}


eqOgre::TransformationEvent &
eqOgre::TransformationEvent::operator=(const eqOgre::TransformationEvent& a)
{
	_node = a._node;
	_last_time = a._last_time;
	_speed = a._speed;
	_angular_speed = a._angular_speed;
	_move_keys = a._move_keys;
	_rot_keys = a._rot_keys;

	// TODO Should these be copied, they are temporary variables in a way
//	_move_dir = a._move_dir;
//	_rotation_axises = a._rotation_axises;
	return *this;
}

void eqOgre::TransformationEvent::operator()(void )
{
	clock_t time = ::clock();
	// Secs since last frame
	double t = ((double)( time - _last_time ))/CLOCKS_PER_SEC;

	// Check that we have an object and we are moving
	// TODO should check the Vector3 with a delta zero
	if( _node && _move_dir != Ogre::Vector3::ZERO )
	{
		Ogre::Vector3 pos = _node->getPosition();
		pos += _speed*t*_move_dir.normalisedCopy();
		_node->setPosition( pos );
	}

	if( _node && _rotation_axises != Ogre::Vector3::ZERO )
	{
		Ogre::Quaternion orient = _node->getOrientation();
		Ogre::Quaternion qx( _angular_speed*t, _rotation_axises );
		_node->setOrientation( qx*orient );
	}

	_last_time = time;
}

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

