/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 */

#include "config_events.hpp"

#include "game_manager.hpp"
#include "player.hpp"

/// -------- ConfigAction ------------

// std::ostream &
// eqOgre::ConfigOperation::print(std::ostream& os) const
// {
// 	os << *this << " config = " << _config << std::endl;
// 	return os;
// }

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

std::string const &
eqOgre::HeadTrackerAction::getTypeName( void ) const
{ return HeadTrackerActionFactory::TYPENAME; }

const std::string eqOgre::HeadTrackerActionFactory::TYPENAME = "HeadTrackerAction";

/// -------- QuitAction ------------

void
eqOgre::QuitAction::execute( void )
{
	if( !_game )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_game->quit();
}

std::string const &
eqOgre::QuitAction::getTypeName( void ) const
{ return eqOgre::QuitActionFactory::TYPENAME; }

const std::string eqOgre::QuitActionFactory::TYPENAME = "QuitAction";


/// -------- ReloadScene ------------
void
eqOgre::ReloadScene::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	data->reloadScene();
}

std::string const &
eqOgre::ReloadScene::getTypeName( void ) const
{ return eqOgre::ReloadSceneFactory::TYPENAME; }

const std::string eqOgre::ReloadSceneFactory::TYPENAME = "ReloadScene";


/// -------- ToggleMusic ------------
void
eqOgre::ToggleMusic::execute(void )
{
	if( !_game )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_game->toggleBackgroundSound();
}

std::string const &
eqOgre::ToggleMusic::getTypeName( void ) const
{ return eqOgre::ToggleMusicFactory::TYPENAME; }


const std::string eqOgre::ToggleMusicFactory::TYPENAME = "ToggleMusic";




/// -------- ActivateCamera ------------
void
eqOgre::ActivateCamera::execute( void )
{
	if( !data )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

 	data->setActiveCamera( _camera_name );
}

std::string const &
eqOgre::ActivateCamera::getTypeName( void ) const
{ return eqOgre::ActivateCameraFactory::TYPENAME; }

const std::string eqOgre::ActivateCameraFactory::TYPENAME = "ActivateCamera";



/// -------- AddTransformOperation ------------
void
eqOgre::AddTransformOperation::execute( void )
{
	if( !_event_man )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_event_man->addEvent( _transform );
}

std::string const &
eqOgre::AddTransformOperation::getTypeName( void ) const
{ return eqOgre::AddTransformOperationFactory::TYPENAME; }

const std::string eqOgre::AddTransformOperationFactory::TYPENAME = "AddTransformOperation";


/// -------- RemoveTransformOperation ------------
void
eqOgre::RemoveTransformOperation::execute(void )
{
	if( !_event_man )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_event_man->removeEvent( _transform );
}

std::string const &
eqOgre::RemoveTransformOperation::getTypeName( void ) const
{ return eqOgre::RemoveTransformOperationFactory::TYPENAME; }

const std::string eqOgre::RemoveTransformOperationFactory::TYPENAME = "RemoveTransformOperation";
