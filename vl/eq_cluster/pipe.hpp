/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

#ifndef EQ_ORGE_PIPE_HPP
#define EQ_ORGE_PIPE_HPP

#include <eq/client/pipe.h>

#include "eq_ogre/ogre_root.hpp"
#include "eq_settings.hpp"
#include "player.hpp"
#include "distrib_resource_manager.hpp"
#include "scene_manager.hpp"

#include "cluster/client.hpp"
#include "session.hpp"

namespace eqOgre
{

class Pipe : public eq::Pipe, vl::Session
{
public :
	Pipe( eq::Node *parent );

	virtual ~Pipe( void );

	DistributedSettings const &getSettings( void ) const;

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

protected :
	/// Equalizer overrides

	/// Override configInit to map the distributed data
	virtual bool configInit( const eq::uint128_t& initID );

	/// Override configExit to unmap the distributed data
	virtual bool configExit();

	/// Override frameStart to update the distributed data
	virtual void frameStart( const eq::uint128_t& frameID,
							 const uint32_t frameNumber );

	/// Ogre helpers
	bool _createOgre( void );
	bool _loadScene( void );

	/// message passing
	void _createClient( void );
	void _handleMessages( void );
	void _handleMessage( vl::cluster::Message *msg );

	/// Distribution helpers
	void _syncData( void );
	void _mapData( eq::uint128_t const &settingsID);
	void _updateDistribData( void );

	/// Input events
	void _sendEvents( void );

	/// Ogre data
	vl::ogre::RootRefPtr _root;
	Ogre::SceneManager *_ogre_sm;
	Ogre::Camera *_camera;


	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	eqOgre::DistributedSettings _settings;
	vl::DistribResourceManager _resource_manager;
	vl::Player _player;
	std::string _active_camera_name;
	uint32_t _screenshot_num;

	vl::cluster::Client *_client;
	std::vector<vl::cluster::ObjectData> _objects;

	/// Input events to be sent
	std::vector<vl::cluster::EventData> _events;

};	// class Pipe

}	// namespace eqOgre

#endif // EQ_ORGE_PIPE_HPP
