#ifndef VL_UDP_OGRE_COMMAND_HPP
#define VL_UDP_OGRE_COMMAND_HPP

#include "command.hpp"
#include <OGRE/OgreSceneNode.h>
#include <base/typedefs.hpp>

namespace vl
{
	
namespace udp
{

class OgreCommand : public vl::udp::Command
{
public :
	OgreCommand( Ogre::SceneNode *node );
	
	virtual double &at( size_t i )
	{ return _data.at(i); }

	virtual double const &at( size_t i ) const
	{ return _data.at(i); }

	static OgreCommand *create( std::string const &cmd_name, Ogre::SceneNode *node );

	static OgreCommand *create( CMD_TYPE cmd_type, Ogre::SceneNode *node );
	
protected :
	Ogre::SceneNode *_node;
	std::vector<double> _data;

};	// class OgreCommand

class SetPosition : public OgreCommand
{
public :
	SetPosition( Ogre::SceneNode *node );
	
	/// Execute the command
	virtual void operator()( void );

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 3; }

	virtual CMD_TYPE getType( void ) const
	{ return CMD_POS; }

	virtual std::vector<double> &operator<<( std::vector<double> &vec );
};

class SetQuaternion : public OgreCommand
{
public :
	SetQuaternion( Ogre::SceneNode *node );
	
	/// Execute the command
	virtual void operator()( void );

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 4; }
	
	virtual CMD_TYPE getType( void ) const
	{ return CMD_ROT_QUAT; }

	virtual std::vector<double> &operator<<( std::vector<double> &vec );
};

class SetAngle : public OgreCommand
{
public :
	SetAngle( Ogre::SceneNode *node );
	
	/// Execute the command
	virtual void operator()( void );

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const
	{ return 4; }
	
	virtual CMD_TYPE getType( void ) const
	{ return CMD_ROT_ANGLE; }

	virtual std::vector<double> &operator<<( std::vector<double> &vec );
};

class SetAngleAxis : public OgreCommand
{
public :
	SetAngleAxis( Ogre::SceneNode *node );
	
	/// Execute the command
	virtual void operator()( void );

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
