/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file action.cpp
 *
 *	Event Handling Action class
 */

#ifndef VL_ACTION_HPP
#define VL_ACTION_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

// Necessary for vl::scalar
#include "math/math.hpp"
// Necessary for Transform
#include "math/transform.hpp"
// Necessary for timed action
#include "base/timer.hpp"

namespace vl
{

/// Actions

/// Action is divided into different Action types depending on the input parameters for execute
/// Simple ones with no parameters
/// and more complex like transformation operations which need the Transformation
class Action
{
public :
	virtual std::string getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << "Operation of type = " << getTypeName() << std::endl;
		return os;
	}

};

typedef Action * ActionPtr;

inline std::ostream &operator<<( std::ostream &os, Action const &a )
{ return a.print(os); }


/// The most basic action, takes no parameters
class BasicAction : public Action
{
public :
	virtual void execute( void ) = 0;
};

typedef BasicAction * BasicActionPtr;


/// Action proxy with two states: on and off
/// Depending on the current state executes different a action
///
/// This class is not ment to be inherited from it's a simple proxy class
/// If you need a similar one create a new one
class ToggleActionProxy : public BasicAction
{
public :
	ToggleActionProxy( void )
		: _action_on(0), _action_off(0), _state(false)
	{}

	void setActionOn( BasicActionPtr action )
	{ _action_on = action; }

	BasicActionPtr getActionOn( void )
	{ return _action_on; }

	void setActionOff( BasicActionPtr action )
	{ _action_off = action; }

	BasicActionPtr getActionOff( void )
	{ return _action_off; }

	void execute( void );

	static ToggleActionProxy *create( void )
	{ return new ToggleActionProxy; }

	std::string getTypeName( void ) const
	{ return "ToggleActionProxy"; }

private :
	BasicActionPtr _action_on;
	BasicActionPtr _action_off;
	bool _state;

};	// class ToggleActionProxy

/** @class BufferActionProxy
 *	@brief Ring buffer that executes the next action in the buffer
 */
class BufferActionProxy : public BasicAction
{
public :
	BufferActionProxy( void )
		: _index(0)
	{}

	void execute( void );

	void addAction( BasicActionPtr action )
	{
		_buffer.push_back(action);
	}

	void remAction( BasicActionPtr action );

	BasicActionPtr getAction( size_t index );

	static BufferActionProxy *create( void )
	{ return new BufferActionProxy; }

	std::string getTypeName( void ) const
	{ return "BufferActionProxy"; }

private :
	std::vector<BasicActionPtr> _buffer;
	size_t _index;

};	// class BufferActionProxy

/** @class GroupActionProxy
 *	@brief Groups multiple actions together so that they are executed when ever the group is
 */
class GroupActionProxy : public BasicAction
{
public :
	GroupActionProxy( void ) {}

	void execute( void );

	void addAction(BasicActionPtr action);

	void remAction(BasicActionPtr action);

	BasicActionPtr getAction(size_t i);

	size_t nActions(void) const;

	bool hasAction(BasicActionPtr action) const;

	static GroupActionProxy *create( void )
	{ return new GroupActionProxy; }

	std::string getTypeName( void ) const
	{ return "GroupActionProxy"; }

private :
	std::vector<BasicActionPtr> _actions;

};	// GroupActionProxy

typedef GroupActionProxy * GroupActionProxyPtr;

/// Action proxy with a timer and time limit
/// Depending on wether the enough time has passed since last activation
/// executes an action or is a NOP
///
/// This class is not ment to be inherited from it's a simple proxy class
/// If you need a similar one create a new one
class TimerActionProxy : public BasicAction
{
public :
	TimerActionProxy( void );

	void setAction( BasicActionPtr action )
	{ _action = action; }

	BasicActionPtr getAction( void )
	{ return _action; }

	/// Set the time limit between the same actions beign triggered
	/// parameter time_limit in seconds
	/// @todo change to using time structure
	void setTimeLimit( double time_limit )
	{ _time_limit = time_limit; }

	/// Get the time limit between actions beign triggered
	/// returns the time limit in seconds
	/// @todo change to using time structure
	double getTimeLimit( void ) const
	{ return _time_limit; }

	void execute(void);

	static TimerActionProxy *create( void )
	{ return new TimerActionProxy; }

	std::string getTypeName( void ) const
	{ return "TimerActionProxy"; }

private :
	BasicActionPtr _action;

	vl::timer _clock;

	/// Time limit in seconds, easier to compare to clock output
	/// @todo replace with time structure
	double _time_limit;
};

class FloatAction : public Action
{
public :
	virtual void execute( vl::scalar const &data ) = 0;
};

typedef FloatAction *FloatActionPtr;

class VectorAction : public Action
{
public :
	virtual void execute( Ogre::Vector3 const &data ) = 0;
};

typedef VectorAction *VectorActionPtr;

/// Callback Action class designed for Trackers
/// Could be expanded for use with anything that sets the object transformation
// For now the Tracker Triggers are the test case
class TransformAction : public Action
{
public :
	/// Callback function for TrackerTrigger
	/// Called when new data is received from the tracker
	virtual void execute( Transform const &data ) = 0;

};

typedef TransformAction * TransformActionPtr;

/// Convert a BasicAction to a FloatAction with a specific value
class FloatActionMap : public BasicAction
{
public :
	FloatActionMap( void )
		: value(0)
	{}

	void execute( void )
	{
		if( _action )
		{ _action->execute(value); }
	}

	void setAction( FloatActionPtr action )
	{ _action = action; }

	FloatActionPtr getAction( void )
	{ return _action; }

	static FloatActionMap *create( void )
	{ return new FloatActionMap; }

	std::string getTypeName( void ) const
	{ return "FloatActionMap"; }

	vl::scalar value;

private :
	FloatActionPtr _action;
};

/// Convert a FloatAction to a VectorAction with a specific value
class VectorActionMap : public FloatAction
{
public :
	VectorActionMap( void )
		: axis( Ogre::Vector3::ZERO )
	{}

	void execute( vl::scalar const &data )
	{
		if( _action )
		{ _action->execute( data*axis ); }
	}

	void setAction( VectorActionPtr action )
	{ _action = action; }

	VectorActionPtr getAction( void )
	{ return _action; }

	static VectorActionMap *create( void )
	{ return new VectorActionMap; }

	std::string getTypeName( void ) const
	{ return "VectorActionMap"; }

	Ogre::Vector3 axis;

private :
	VectorActionPtr _action;
};


}	// namespace vl

#endif // VL_ACTION_HPP
