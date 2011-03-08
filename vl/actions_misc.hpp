/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 *	2011-01 Dependencies to Equalizer removed and moved to more suitable place
 */

#ifndef VL_ACTIONS_MISC_HPP
#define VL_ACTIONS_MISC_HPP

#include "tracker.hpp"
#include "action.hpp"

#include "typedefs.hpp"

namespace vl
{

template< typename T >
class ActionBase : public vl::BasicAction
{
public :
	ActionBase( void )
		: data(0)
	{}

	T *data;

};	// class ActionBase

typedef ActionBase<Player> PlayerAction;
typedef ActionBase<GameManager> GameAction;
typedef ActionBase<SceneManager> SceneManagerAction;

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

class ScreenshotAction : public PlayerAction
{
public :
	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ScreenshotAction"; }

	static ScreenshotAction *create( void )
	{ return new ScreenshotAction; }

};	// ScreenshotAction

class ReloadScene : public SceneManagerAction
{
public :
	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ReloadScene"; }

	static ReloadScene *create( void )
	{ return new ReloadScene; }

};

class AddToSelection : public SceneManagerAction
{
public :
	AddToSelection( void )
		: _node(0)
	{}

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "AddToSelection"; }

	static AddToSelection *create( void )
	{ return new AddToSelection; }

	void setSceneNode(SceneNodePtr node)
	{ _node = node; }

	SceneNodePtr getSceneNode(void)
	{ return _node; }

private :
	SceneNodePtr _node;
};

class RemoveFromSelection : public SceneManagerAction
{
public :
	RemoveFromSelection( void )
		: _node(0)
	{}

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "RemoveFromSelection"; }

	static RemoveFromSelection *create( void )
	{ return new RemoveFromSelection; }

	void setSceneNode(SceneNodePtr node)
	{ _node = node; }

	SceneNodePtr getSceneNode(void)
	{ return _node; }

private :
	SceneNodePtr _node;
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


}	// namespace vl

#endif	// VL_ACTIONS_MISC_HPP
