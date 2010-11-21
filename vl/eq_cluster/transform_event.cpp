
#include "transform_event.hpp"

eqOgre::MoveOperation::MoveOperation( void )
	: _node(0), _speed(1), _angular_speed( Ogre::Degree(60) ),
		_move_dir(Ogre::Vector3::ZERO), _rot_dir(Ogre::Vector3::ZERO)
{}

void
eqOgre::MoveOperation::execute(double time)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	// Check that we are moving
	if( !_move_dir.isZeroLength())
	{
		Ogre::Vector3 pos = _node->getPosition();
		pos += _speed*time*_move_dir.normalisedCopy();
		_node->setPosition( pos );
	}

	if( !_rot_dir.isZeroLength() )
	{
		Ogre::Quaternion orient = _node->getOrientation();
		Ogre::Quaternion qx( _angular_speed*time, _rot_dir );
		_node->setOrientation( qx*orient );
	}
}

std::string const eqOgre::MoveOperation::TYPENAME = "MoveOperation";

eqOgre::TransformationEvent::TransformationEvent( void )
	: _last_time( ::clock() ),
	  _operation( new MoveOperation )
{}

std::string const &
eqOgre::TransformationEvent::getTypeName(void ) const
{ return TransformationEventFactory::TYPENAME; }

eqOgre::TransformationEvent::TransformationEvent(const eqOgre::TransformationEvent& a)
{
	*this = a;
}


eqOgre::TransformationEvent &
eqOgre::TransformationEvent::operator=(const eqOgre::TransformationEvent& a)
{
	// TODO copying is disallowed for now
	return *this;
}

// FIXME
std::ostream &
eqOgre::operator<<(std::ostream& os, const eqOgre::TransformationEvent& a)
{
/*
	std::string del("     ");

	if( a.getSceneNode() )
		os << "TransformationEvent for node = " << a.getSceneNode()->getName();
	else
		os << "TransformationEvent without node";
	os << std::endl
		<< del << "speed = " << a.getSpeed()
		<< " m/s : " << " angular speed = " << a.getAngularSpeed() << std::endl;
//		<< del << "move keys = " << a._move_keys << std::endl
//		<< del << "rot keys = " << a._rot_keys;
*/
	return os;
}


bool
eqOgre::TransformationEvent::processTrigger(vl::Trigger* trig)
{
	bool retval = false;
	Ogre::Vector3 v = _trans_triggers.findTrigger(trig);
	if( !v.isZeroLength() )
	{
		_operation->addMove( v );
		retval = true;
	}

	v = _rot_triggers.findTrigger(trig);
	if( !v.isZeroLength() )
	{
		_operation->addRotation( v );
		retval = true;
	}

	if( vl::FrameTrigger().isSimilar(trig) )
	{
		// TODO the delta time should be provided by the FrameTrigger
		clock_t time = ::clock();
		// Secs since last frame
		double t = ((double)( time - _last_time ))/CLOCKS_PER_SEC;

		_operation->execute( t );

		_last_time = time;
		retval = true;
	}

	return retval;
}

std::string const eqOgre::TransformationEventFactory::TYPENAME = "TransformationEvent";
