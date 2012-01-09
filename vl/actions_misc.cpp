/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file actions_misc.cpp
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

#include "actions_misc.hpp"

#include "scene_manager.hpp"
#include "game_manager.hpp"
#include "player.hpp"
#include "ray_object.hpp"

#include "python/python.hpp"

/// ----------------------------- ScriptAction ---------------------------------
void
vl::ScriptAction::execute(void)
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
	data->getPython()->executeCommand(script);
}


/// ----------------------- HeadTrackerAction ----------------------------------
void
vl::HeadTrackerAction::execute( vl::Transform const &data )
{
	if( !_player )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_player->setHeadTransform(data);
}

/// -------- QuitAction ------------

void
vl::QuitAction::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->quit();
}


/// -------- ReloadScene ------------
void
vl::ReloadScene::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->reloadScene();
}

void
vl::AddToSelection::execute( void )
{
	if( !data || !_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	data->addToSelection(_node);
}

void
vl::RemoveFromSelection::execute( void )
{
	if( !data || !_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	data->removeFromSelection(_node);
}

/// -------- ToggleMusic ------------
void
vl::ToggleMusic::execute(void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->toggleBackgroundSound();
}


/// -------- ActivateCamera ------------
void
vl::ActivateCamera::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

 	data->setActiveCamera( _camera_name );
}

/// -------- ScreenshotAction ------------
void
vl::ScreenshotAction::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

 	data->takeScreenshot();
}
