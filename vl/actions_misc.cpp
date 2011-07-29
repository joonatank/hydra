/**	@author Joonatan Kuosa
 *	@date 2010-11 initial implementation
 *	@file actions_misc.cpp
 */

#include "actions_misc.hpp"

#include "scene_manager.hpp"
#include "game_manager.hpp"
#include "player.hpp"
#include "ray_object.hpp"

#include "python.hpp"

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
