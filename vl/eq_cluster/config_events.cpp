/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 */

#include "config_events.hpp"

#include "game_manager.hpp"
#include "player.hpp"

/// --------- HeadTrackerAction -----------
void
eqOgre::HeadTrackerAction::execute( vl::Transform const &data )
{
	if( !_player )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	Ogre::Matrix4 m( data.quaternion );
	m.setTrans(data.position);
	_player->setHeadMatrix(m);
}

/// -------- QuitAction ------------

void
eqOgre::QuitAction::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->quit();
}


/// -------- ReloadScene ------------
void
eqOgre::ReloadScene::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->reloadScene();
}


/// -------- ToggleMusic ------------
void
eqOgre::ToggleMusic::execute(void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->toggleBackgroundSound();
}


/// -------- ActivateCamera ------------
void
eqOgre::ActivateCamera::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

 	data->setActiveCamera( _camera_name );
}
