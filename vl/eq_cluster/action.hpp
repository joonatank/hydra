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