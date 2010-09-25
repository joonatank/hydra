#ifndef VL_UDP_OGRE_COMMAND_HPP
#define VL_UDP_OGRE_COMMAND_HPP

#include "command.hpp"
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <base/typedefs.hpp>

namespace vl
{
	
namespace udp
{

class OgreCommand : public vl::udp::Command
{
public :
	OgreCommand( Ogre::SceneNode *node );

	virtual ~OgreCommand( void ) {}

	static OgreCommand *create( std::string const &cmd_name, Ogre::SceneNode *node );

	static OgreCommand *create( CMD_TYPE cmd_type, Ogre::SceneNode *node );
	
protected :
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

	virtual std::vector<double> &operator<<( std::vector<double> &vec );

	virtual std::ostream &operator<<( std::ostream &os ) const;
	
	Ogre::Vector3 &getPosition( void )
	{ return _pos; }

	Ogre::Vector3 const &getPosition( void ) const
	{ return _pos; }

	void const setPosition( Ogre::Vector3 const &pos )
	{ _pos = pos; }
	
protected :
	Ogre::Vector3 _pos;
};

class SetOrientation : public OgreCommand
{
public :
	SetOrientation( Ogre::SceneNode *node );

	virtual ~SetOrientation( void ) {}
	
	/// Execute the command
	virtual void operator()( void );

	virtual std::ostream &operator<<( std::ostream &os ) const;

	Ogre::Quaternion &getQuaternion( void )
	{ return _quat; }

	Ogre::Quaternion const &getQuaternion( void ) const
	{ return _quat; }

	void setQuaternion( Ogre::Quaternion const &rot )
	{ _quat = rot; }
	
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

	virtual std::vector<double> &operator<<( std::vector<double> &vec );
};

class SetAngle : public SetOrientation
{
public :
	SetAngle( Ogre::SceneNode *node );

    virtual ~SetAngle( void ) {}

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 4; }
	
	virtual CMD_TYPE getType( void ) const
	{ return CMD_ROT_ANGLE; }

	virtual std::vector<double> &operator<<( std::vector<double> &vec );
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

	virtual std::vector<double> &operator<<( std::vector<double> &vec );
};

}	// namespace udp

}	// namespace vl

#endif // VL_UDP_OGRE_COMMAND_HPP
