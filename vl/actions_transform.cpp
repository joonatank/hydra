/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */

// Interface header
#include "actions_transform.hpp"

//#include "keycode.hpp"
#include "base/exceptions.hpp"

#include "scene_node.hpp"
#include "scene_manager.hpp"

vl::MoveAction::MoveAction( void )
	: _move_dir(Ogre::Vector3::ZERO)
	, _rot_dir(Ogre::Vector3::ZERO)
	, _speed(1)
	, _angular_speed( Ogre::Degree(60) )
	, _local(true)
{}

void
vl::MoveAction::execute( void )
{
	double time = (double)(_clock.getMicroseconds())/1e6;
	if( !_move_dir.isZeroLength() )
	{
		move( _move_dir*_speed*time, _local );
	}

	if( !_rot_dir.isZeroLength() )
	{
		Ogre::Quaternion qx( _angular_speed*time, _rot_dir );
		rotate(qx, _local);
	}

	_clock.reset();
}


vl::MoveNodeAction::MoveNodeAction( void )
	: _node(0)
{}

void 
vl::MoveNodeAction::move(Ogre::Vector3 const &v, bool local)
{
	// TODO replace with error reporting
	assert(_node);
	Ogre::Vector3 mov;
	if(local)
	{ mov = _node->getOrientation() * v; }
	else
	{ mov = v; }

	_node->setPosition(_node->getPosition() + mov);
}

void 
vl::MoveNodeAction::rotate(Ogre::Quaternion const &q, bool local)
{
	// TODO replace with error reporting
	assert(_node);

	Ogre::Quaternion orient = _node->getOrientation();
	_node->setOrientation( q*orient );
}

vl::MoveSelectionAction::MoveSelectionAction( void )
	: _scene(0)
{}

void 
vl::MoveSelectionAction::move(Ogre::Vector3 const &v, bool local)
{
	// TODO replace with error reporting
	assert(_scene);

	SceneNodeList const &list = _scene->getSelection();
	SceneNodeList::const_iterator iter;
	for( iter = list.begin(); iter != list.end(); ++iter )
	{
		Ogre::Vector3 mov;
		if(local)
		{ mov = (*iter)->getOrientation() * v; }
		else
		{ mov = v; }

		(*iter)->setPosition((*iter)->getPosition() + mov);
	}
}
	
void 
vl::MoveSelectionAction::rotate(Ogre::Quaternion const &q, bool local)
{
	// TODO replace with error reporting
	assert(_scene);

	SceneNodeList const &list = _scene->getSelection();
	SceneNodeList::const_iterator iter;
	for( iter = list.begin(); iter != list.end(); ++iter )
	{
		Ogre::Quaternion orient = (*iter)->getOrientation();
		(*iter)->setOrientation( q*orient );
	}
}
