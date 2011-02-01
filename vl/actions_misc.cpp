/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 */

#include "actions_misc.hpp"

#include "scene_manager.hpp"
#include "game_manager.hpp"
#include "player.hpp"

/// --------- HeadTrackerAction -----------
void
vl::HeadTrackerAction::execute( vl::Transform const &data )
{
	if( !_player )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	Ogre::Matrix4 m( data.quaternion );
	m.setTrans(data.position);
	_player->setHeadMatrix(m);
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
