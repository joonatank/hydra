/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */

// Interface header
#include "actions_transform.hpp"

vl::MoveAction::MoveAction( void )
	: _node(0), _move_dir(Ogre::Vector3::ZERO),
		_rot_dir(Ogre::Vector3::ZERO),
		_speed(1), _angular_speed( Ogre::Degree(60) )
{}

void
vl::MoveAction::execute( void )
{
	double time = (double)(_clock.getMicroseconds())/1e6;
	if( _node && !_move_dir.isZeroLength() )
	{
		Ogre::Vector3 v = _node->getOrientation() *  _move_dir.normalisedCopy();
		_node->setPosition( _node->getPosition()+ v*time*_speed );
	}

	if( _node && !_rot_dir.isZeroLength() )
	{
		Ogre::Quaternion orient = _node->getOrientation();
		Ogre::Quaternion qx( _angular_speed*time, _rot_dir );
		_node->setOrientation( qx*orient );
	}

	_clock.reset();
}
