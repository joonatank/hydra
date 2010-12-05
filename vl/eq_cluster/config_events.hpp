/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 */

#ifndef EQ_OGRE_CONFIG_EVENTS_HPP
#define EQ_OGRE_CONFIG_EVENTS_HPP

#include "event.hpp"
#include "config.hpp"
#include "tracker.hpp"

#include "base/typedefs.hpp"

namespace eqOgre
{

template< typename T >
class ActionBase : public vl::BasicAction
{
public :
	ActionBase( void )
		: data(0)
	{}

// 	void setData( T *data )
// 	{ _data = data; }
//
// 	Config *getConfig( void )
// 	{ return _config; }

// 	virtual std::ostream & print( std::ostream & os ) const;

// protected :
	T *data;

};	// class ConfigOperation

typedef ActionBase<vl::Player> PlayerAction;

/// Sets the Head matrix in eqOgre::Config
// TODO this should use the same PlayerAction base
// Problem is that it inherits from BasicAction and this inherits from TransformAction
class HeadTrackerAction : public vl::TransformAction
{
public :
	HeadTrackerAction( void )
		: _player(0)
	{}

	void setPlayer( vl::PlayerPtr player )
	{ _player = player; }

	vl::PlayerPtr getPlayer( void )
	{ return _player; }

	/// Callback function for TrackerTrigger
	/// Called when new data is received from the tracker
	/// Sets the head matrix in Config
	virtual void execute( vl::Transform const &data );

	virtual std::string const &getTypeName( void ) const;

protected :
	vl::PlayerPtr _player;
};

class HeadTrackerActionFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new HeadTrackerAction; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

class GameAction : public vl::BasicAction
{
public :
	GameAction( void )
		: _game(0)
	{}

	void setGame( vl::GameManagerPtr game )
	{ _game = game; }

	vl::GameManagerPtr getGame( void )
	{ return _game; }

protected :
	vl::GameManagerPtr _game;
};

class QuitAction : public GameAction
{
public :
	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

};	// class QuitOperation

class QuitActionFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new QuitAction; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

class ActivateCamera : public PlayerAction
{
public :
	void setCamera( std::string const &name )
	{ _camera_name = name; }

	std::string const &getCamera( void ) const
	{ return _camera_name; }

	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

protected :
	std::string _camera_name;

};	// ActivateCamera

class ActivateCameraFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new ActivateCamera; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

};	// ActivateCameraFactory


typedef ActionBase<SceneManager> SceneManagerAction;

class ReloadScene : public SceneManagerAction
{
public :
	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

};

class ReloadSceneFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new ReloadScene; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


class ToggleMusic : public GameAction
{
public :
	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

};

class ToggleMusicFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new ToggleMusic; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};







class EventManagerOperation : public vl::BasicAction
{
public :
	EventManagerOperation( void )
		: _event_man(0)
	{}

	void setManager( vl::EventManager *event_man )
	{ _event_man = event_man; }

	vl::EventManager *getManager( void )
	{ return _event_man; }

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << *this << " event manager = " << _event_man << std::endl;
		return os;
	}

protected :
	vl::EventManager *_event_man;
};



class AddTransformOperation : public EventManagerOperation
{
public :
	AddTransformOperation( void )
		: _transform(0)
	{}

	void setTransformEvent( TransformationEvent *transform )
	{ _transform = transform; }

	TransformationEvent *getTransformEvent( void )
	{ return _transform; }

	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

private :
	TransformationEvent *_transform;

};

class AddTransformOperationFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new AddTransformOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};



class RemoveTransformOperation : public EventManagerOperation
{
public :
	RemoveTransformOperation( void )
		: _transform(0)
	{}

	void setTransformEvent( TransformationEvent *transform )
	{ _transform = transform; }

	TransformationEvent *getTransformEvent( void )
	{ return _transform; }


	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

private :
	TransformationEvent *_transform;

};

class RemoveTransformOperationFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new RemoveTransformOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


}	// namespace eqOgre

#endif	// EQ_OGRE_CONFIG_EVENTS_HPP