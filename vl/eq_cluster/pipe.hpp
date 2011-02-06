/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

#ifndef EQ_ORGE_PIPE_HPP
#define EQ_ORGE_PIPE_HPP

#include "eq_ogre/ogre_root.hpp"
#include "base/envsettings.hpp"
#include "player.hpp"
#include "scene_manager.hpp"

#include "cluster/client.hpp"
#include "session.hpp"
#include "settings.hpp"

namespace eqOgre
{

class GLWindow;
class Window;

class Pipe : public vl::Session
{
public :
	Pipe( std::string const &name,
		  std::string const &server_address,
		  uint16_t server_port );

	virtual ~Pipe( void );

	vl::EnvSettingsRefPtr getSettings( void );

	vl::EnvSettings::Node getNodeConf( void );

	vl::EnvSettings::Window getWindowConf( std::string const &window_name );

	vl::ogre::RootRefPtr getRoot( void )
	{ return _root; }

	Ogre::SceneManager *getSceneManager( void )
	{ return _ogre_sm; }

	Ogre::Camera *getCamera( void )
	{ return _camera; }

	vl::Player const &getPlayer( void ) const
	{ return _player; }

	/// Send a message to the Master
	/// Does not destroy the message so user is responsible for destroying it.
	void sendMessageToMaster( vl::cluster::Message *msg );

	void sendEvent( vl::cluster::EventData const &event );

	/// Boost thread operator
	void operator()();

	std::string const &getName( void ) const
	{ return _name; }

	typedef std::vector<vl::ProjSettingsRefPtr> ProjectList;

protected :
	/// Reload the projects
	void _reloadProjects( vl::Settings set );

	/// Ogre helpers
	void _createOgre( void );
	void _loadScene( vl::ProjSettings::Scene const &scene );
	void _setCamera( void );

	/// message passing
	void _createClient( std::string const &server_address, uint16_t server_port );
	void _handleMessages( void );
	void _handleMessage( vl::cluster::Message *msg );
	void _handleUpdateMsg( vl::cluster::Message *msg );

	/// Distribution helpers
	/// Syncs to the master copy stored when an Update message was received
	void _syncData( void );
	/// Maps all distributed objects i.e. player and SceneManager
	/// Scene Manager handles later mapping of SceneNodes
	void _mapData( void );
	/// Updates command data, uses versioning to and indexes to provide an
	/// Commands that can be sent from Master thread
	void _updateDistribData( void );

	/// Rendering helpers
	/// Will render all the windows
	void _draw( void );
	/// Will swap all the windows
	void _swap( void );

	/// Input events
	void _sendEvents( void );

	void _createWindow( vl::EnvSettings::Window const &winConf );

	std::string _name;

	/// EnvSettings mapped from Master
	vl::EnvSettingsRefPtr _env;
	/// ProjectSettings mapped from Master
	vl::Settings _settings;

	/// Ogre data
	vl::ogre::RootRefPtr _root;
	Ogre::SceneManager *_ogre_sm;
	Ogre::Camera *_camera;


	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	vl::Player _player;
	std::string _active_camera_name;
	uint32_t _screenshot_num;

	vl::cluster::Client *_client;
	std::vector<vl::cluster::ObjectData> _objects;

	/// Input events to be sent
	std::vector<vl::cluster::EventData> _events;

	std::vector<eqOgre::Window *> _windows;

};	// class Pipe

}	// namespace eqOgre

#endif // EQ_ORGE_PIPE_HPP
