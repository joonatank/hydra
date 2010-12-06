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

	T *data;

};	// class ConfigOperation

typedef ActionBase<vl::Player> PlayerAction;
typedef ActionBase<vl::GameManager> GameAction;

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

	virtual std::string getTypeName( void ) const
	{ return "HeadTrackerAction"; }

	static HeadTrackerAction *create( void )
	{ return new HeadTrackerAction; }

protected :
	vl::PlayerPtr _player;
};


class QuitAction : public GameAction
{
public :
	virtual void execute( void );

	static QuitAction *create( void )
	{ return new QuitAction; }

	virtual std::string getTypeName( void ) const
	{ return "QuitAction"; }

};	// class QuitOperation


class ActivateCamera : public PlayerAction
{
public :
	void setCamera( std::string const &name )
	{ _camera_name = name; }

	std::string const &getCamera( void ) const
	{ return _camera_name; }

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ActivateCamera"; }

	static ActivateCamera *create( void )
	{ return new ActivateCamera; }

protected :
	std::string _camera_name;

};	// ActivateCamera


typedef ActionBase<SceneManager> SceneManagerAction;

class ReloadScene : public SceneManagerAction
{
public :
	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ReloadScene"; }

	static ReloadScene *create( void )
	{ return new ReloadScene; }

};


class ToggleMusic : public GameAction
{
public :
	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ToggleMusic"; }

	static ToggleMusic *create( void )
	{ return new ToggleMusic; }

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

	virtual std::string getTypeName( void ) const
	{ return "AddTransformOperation"; }

	static AddTransformOperation *create( void )
	{ return new AddTransformOperation; }

private :
	TransformationEvent *_transform;

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

	virtual std::string getTypeName( void ) const
	{ return "RemoveTransformOperation"; }

	static RemoveTransformOperation *create( void )
	{ return new RemoveTransformOperation; }

private :
	TransformationEvent *_transform;

};


}	// namespace eqOgre

#endif	// EQ_OGRE_CONFIG_EVENTS_HPP