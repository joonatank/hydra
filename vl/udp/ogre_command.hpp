/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-9
 *
 */

#ifndef VL_UDP_OGRE_COMMAND_HPP
#define VL_UDP_OGRE_COMMAND_HPP

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include <cstring>
#include <vector>

// Base class
#include "command.hpp"

#include "typedefs.hpp"

namespace vl
{

namespace udp
{

class OgreCommand : public vl::udp::Command
{
public :
	OgreCommand( Ogre::SceneNode *node );

	virtual ~OgreCommand( void ) {}

	static CommandRefPtr create( std::string const &cmd_name, Ogre::SceneNode *node );

	static CommandRefPtr create( CMD_TYPE cmd_type, Ogre::SceneNode *node );

	std::vector<double> &operator<<( std::vector<double> &vec )
	{
		check( getSize(), vec );
		copy( getSize(), vec );
		remove( getSize(), vec );
		return vec;
	}

protected :
	/// Abstract methods called from operator<< (deserialization
	/// Checks that there is enough data for deserialization
	virtual void check( size_t size, std::vector<double> &vec );

	/// Deserializises the data
	virtual void copy( size_t size, std::vector<double> &vec ) = 0;

	/// Removes extra elements from the vector
	virtual void remove( size_t size, std::vector<double> &vec );

	Ogre::SceneNode *_node;

};	// class OgreCommand

class SetPosition : public OgreCommand
{
public :
	SetPosition( Ogre::SceneNode *node );

    virtual ~SetPosition( void ) {}

	/// Execute the command
	virtual void operator()( void );

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 3; }

	virtual CMD_TYPE getType( void ) const
	{ return CMD_POS; }

//	virtual std::vector<double> &operator<<( std::vector<double> &vec );

	virtual void print( std::ostream &os ) const;

	Ogre::Vector3 &getPosition( void )
	{ return _pos; }

	Ogre::Vector3 const &getPosition( void ) const
	{ return _pos; }

	void const setPosition( Ogre::Vector3 const &pos )
	{ _pos = pos; }

protected :
	virtual void copy( size_t size, std::vector<double> &vec );

	Ogre::Vector3 _pos;
};

class SetOrientation : public OgreCommand
{
public :
	SetOrientation( Ogre::SceneNode *node );

	virtual ~SetOrientation( void ) {}

	/// Execute the command
	virtual void operator()( void );

	virtual void print( std::ostream &os ) const;

	/// Basic getter setters
	Ogre::Quaternion &getQuaternion( void )
	{ return _quat; }

	Ogre::Quaternion const &getQuaternion( void ) const
	{ return _quat; }

	void setQuaternion( Ogre::Quaternion const &rot )
	{ _quat = rot; }

	virtual void getAngleAxis( Ogre::Radian &angle, Ogre::Vector3 &axis ) const;
	virtual void setAngleAxis( Ogre::Radian const &angle, Ogre::Vector3 const &axis );

protected :
	Ogre::Quaternion _quat;
};

class SetQuaternion : public SetOrientation
{
public :
	SetQuaternion( Ogre::SceneNode *node );

    virtual ~SetQuaternion( void ) {}

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 4; }

	virtual CMD_TYPE getType( void ) const
	{ return CMD_ROT_QUAT; }

protected :
	virtual void copy( size_t size, std::vector<double> &vec );
};

class SetAngle : public SetOrientation
{
public :
	SetAngle( Ogre::SceneNode *node );

    virtual ~SetAngle( void ) {}

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 1; }

	virtual CMD_TYPE getType( void ) const
	{ return CMD_ROT_ANGLE; }

	virtual void operator()( void );

	/// Complex getter setters
	Ogre::Radian getAngle( void ) const;
	void setAngle( Ogre::Radian const &angle );

	Ogre::Vector3 getAxis( void ) const;
	void setAxis( Ogre::Vector3 const &axis );

	virtual void getAngleAxis( Ogre::Radian &angle, Ogre::Vector3 &axis ) const;
	virtual void setAngleAxis( Ogre::Radian const &angle, Ogre::Vector3 const &axis );

protected :
	virtual void copy( size_t size, std::vector<double> &vec );

	Ogre::Vector3 _axis;
	Ogre::Radian _angle;
};

class SetAngleAxis : public SetOrientation
{
public :
	SetAngleAxis( Ogre::SceneNode *node );

    virtual ~SetAngleAxis( void ) {}

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 4; }

	virtual CMD_TYPE getType( void ) const
	{ return CMD_ROT_AA; }

//	virtual std::vector<double> &operator<<( std::vector<double> &vec );
protected :
	virtual void copy( size_t size, std::vector<double> &vec );

};

}	// namespace udp

}	// namespace vl

#endif // VL_UDP_OGRE_COMMAND_HPP
