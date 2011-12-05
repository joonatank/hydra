/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file actions_transform.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/// @depricated Will be removed from next release.

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
	, _reference(0)
{}

void
vl::MoveAction::execute( void )
{
	double time = _clock.elapsed();
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
	if(!_node)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	if(_reference)
	{ _node->translate(v, _reference); }
	else if(local)
	{ _node->translate(v); }
	else
	{ _node->translate(v, TS_WORLD); }
}

void
vl::MoveNodeAction::rotate(Ogre::Quaternion const &q, bool local)
{
	if(!_node)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	// references are not supported for rotations, it's usually not what user wants
	_node->rotate(q);
}

vl::MoveSelectionAction::MoveSelectionAction( void )
	: _scene(0)
{}

void
vl::MoveSelectionAction::move(Ogre::Vector3 const &v, bool local)
{
	if(!_scene)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	SceneNodeList const &list = _scene->getSelection();
	SceneNodeList::const_iterator iter;
	for( iter = list.begin(); iter != list.end(); ++iter )
	{
		if(_reference)
		{ (*iter)->translate(v, _reference); }
		else if(local)
		{ (*iter)->translate(v); }
		else
		{ (*iter)->translate(v, TS_WORLD); }
	}
}

void
vl::MoveSelectionAction::rotate(Ogre::Quaternion const &q, bool local)
{
	if(!_scene)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	SceneNodeList const &list = _scene->getSelection();
	SceneNodeList::const_iterator iter;
	for( iter = list.begin(); iter != list.end(); ++iter )
	{
		// references are not supported for rotations, it's usually not what user wants
		(*iter)->rotate(q);
	}
}
