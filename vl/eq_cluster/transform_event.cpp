
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
		// Transform the moving direction to object coordinates
		Ogre::Vector3 move = _node->getOrientation() * _move_dir.normalisedCopy();
		Ogre::Vector3 pos = _node->getPosition();
		pos += _speed*time*move;
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
	: _operation( new MoveOperation )
{}

std::string const &
eqOgre::TransformationEvent::getTypeName(void ) const
{ return TransformationEventFactory::TYPENAME; }

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
		// Because this is going to go nuts if we miss a couple of FrameTriggers
		double t = _clock.getTimed()/1000;

		_operation->execute( t );

		_clock.reset();
		retval = true;
	}

	return retval;
}

std::string const eqOgre::TransformationEventFactory::TYPENAME = "TransformationEvent";
