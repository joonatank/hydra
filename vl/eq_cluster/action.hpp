/**	Joonatan Kuosa
 *	2010-12
 *
 *	Event Handling Action class
 */

#ifndef VL_ACTION_HPP
#define VL_ACTION_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

namespace vl
{


struct Transform
{
	Transform( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO,
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: position( pos ), quaternion( rot )
	{}

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

inline std::ostream &operator<<( std::ostream &os, Transform const &d )
{
	os << "Position = " << d.position << " : Orientation = " << d.quaternion;

	return os;
}


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

	void execute( void )
	{
		// It's not a real problem if we only have one action but the toggle
		// will not work correctly till we have the other one
		if( _state && _action_off )
		{
			_action_off->execute();
			_state = !_state;
		}
		else if( !_state && _action_on )
		{
			_action_on->execute();
			_state = !_state;
		}
	}

	static ToggleActionProxy *create( void )
	{ return new ToggleActionProxy; }

	std::string getTypeName( void ) const
	{ return "ToggleActionProxy"; }

private :
	BasicActionPtr _action_on;
	BasicActionPtr _action_off;
	bool _state;
};

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

}	// namespace vl

#endif // VL_ACTION_HPP