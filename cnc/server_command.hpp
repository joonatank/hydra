#ifndef VL_SERVER_COMMAND_HPP
#define VL_SERVER_COMMAND_HPP

#include "command.hpp"

#include "interface/root.hpp"

namespace vl
{

namespace server 
{
	// For now we'll use Strings (object names, types), later implement
	// conversion from Strings to ids and back again... but as Ogre uses
	// Strings for creation, deletion and object retrieval it's easier to
	// adopt that.
	//
	// TODO explore possibility for using the same data in all commands or
	// using a map with enum indexes for saving data.
	// This would provide easier transfer and execution of commands and
	// we can just discard data that is not there, like say if the object
	// does not have name, position or rotation 
	// we don't have to transmit that over the network.
	// Will have somewhat of a memory hit compared to different size commands
	// or using dynamic map (which will have heap allocation performance hit).
	// For std::string memory hit is 8 bytes on 64-bit OS.
	// For integers and doubles it's 4 bytes.
	// So for Vector 12 bytes, quat 16 bytes.
	// - Probably usefull to implement, easier Transformation commands and
	// we can transform objects when we create them (just need to do packing
	// when sending over network). To avoid sending empty objects.
	// - Will not help us with the creation problem as the objects have to be
	// created with the exact type.
	// 	->  For the creation problem we need to use proxy to get rid of the
	// 		clutter in serialize(), if we think it's a such a problem.
	//
	//
	// TODO we need to divide the typeName to two, one defines the 
	// inherited master class (or abstract class)
	// like SceneManager, SceneNode, MovableObject etc.
	// And the other one defines the exact class we like to create.
	//
	// FIXME Commands should be divided into two different concrete
	// implementation one has client side and the other has server side.
	// Basicly the client is not interested how the commands are run
	// but this needs to be implemented in somewhere.
	//
	// So basicly we need two different hierarchies (both deriving from
	// Command or Message), server::Command and client::Command.
	// Ah well we can also implement the same data for all commands then...
	class Command : public vl::base::Command
	{
		public : 
			Command( vl::base::CommandStruct const & c
					= vl::base::CommandStruct() )
				: cmd(c)
			{}

			Command( vl::base::CMD_TYPE ctype,
					 std::string const &obj,
					 std::string const &type,
					 std::string const &creat
					 )
			{
				cmd.cmdType = ctype;
				cmd.name = obj;
				cmd.typeName = type;
				cmd.creator = creat;
			}

			vl::base::CommandStruct cmd;

			// Function operator
			virtual void operator()( vl::graph::Root *root ) = 0;
	};

	inline
	std::ostream &operator<<( std::ostream &os, vl::server::Command const &cmd )
	{
		os << cmd.cmd << std::endl;
		return os;
	}
	// Adding Ogre::quaternion and ogre::vector would result in min size of
	// 8*4+sizeof(map)+3*4+4*4 = 60+sizeof(map) ~= 72 bytes
	// Which means that a buffer of 128 Commands would be 9kbytes compared to
	// 5.5kbytes for the same objects without Vector and Quaternion.
	//
	// This does not include the real cost of strings that have data,
	// which would probably be ~24kbytes for 128 objects
	// (assuming 3 strings with 16 chars each).
	class CreateCmd : public vl::server::Command
	{
		public :
			CreateCmd( vl::base::CommandStruct const & c )
				: Command(c)
			{}

			CreateCmd( std::string const &obj,
					   std::string const &type,
					   std::string const &par = std::string(),
					   vl::NamedValuePairList const &param
						   = vl::NamedValuePairList(),
					   std::string const &creat = std::string ()
					   )
				: Command() 
			{
				cmd.cmdType = vl::base::CMD_CREATE;
				cmd.name = obj;
				cmd.typeName = type;
				cmd.creator = creat;
				cmd.parent = par;
				cmd.params = param;
			}

			// Function operator
			virtual void operator()( vl::graph::Root *root );
	};

	class DeleteCmd : public vl::server::Command
	{
		public :
			DeleteCmd( vl::base::CommandStruct const & c )
				: Command(c)
			{}

			DeleteCmd( std::string const &obj,
					   std::string const &typ,
					   std::string const &creat = std::string() )
				: Command( )
			{
				cmd.cmdType = vl::base::CMD_DELETE;
				cmd.name = obj;
				cmd.typeName = typ;
				cmd.creator = creat;
			}

			// Function operator
			virtual void operator()( vl::graph::Root *root );
	};

	class MoveCmd : public vl::server::Command
	{
		public :
			MoveCmd( vl::base::CommandStruct const & c )
				: Command(c)
			{}

			MoveCmd( std::string const &obj,
					 std::string const &typ,
					 vmml::vec3d const &vec = vmml::vec3d::ZERO,
					 vmml::quaterniond const &quat
						 = vmml::quaterniond::IDENTITY,
					 std::string const &creat = std::string()
					 )
			{
				cmd.cmdType = vl::base::CMD_MOVE;
				cmd.name = obj;
				cmd.typeName = typ;
				cmd.creator = creat;
				cmd.position = vec;
				cmd.rotation = quat;
			}

			// Function operator
			virtual void operator()( vl::graph::Root *root );
	};

	/*
	class RotateCmd : public Command
	{
		public :
			// object name to which rotate
			// object type name (Ogre type, like SceneNode or Entity)
			// object creator (Ogre SceneManager)
			// Quaternion representing the rotation
			RotateCmd(  Ogre::String const &obj,
						Ogre::String const &typ,
						Ogre::Quaternion const &q,
						Ogre::String const &creat = Ogre::String() )
				: Command( CMD_ROTATE, obj, typ, creat ), quat(q)
			{}

			// Function operator
			virtual void operator()( Ogre::Root *root );

			Ogre::Quaternion quat;
	};

	class LookAtCmd : public Command
	{
		public :
			LookAtCmd(  Ogre::String const &obj,
						Ogre::String const &typ,
						Ogre::Vector3 const &v,
						Ogre::String const &creat = Ogre::String() )
				: Command( CMD_LOOKAT, obj, typ, creat ), vec(v)
			{}

			// Function operator
			virtual void operator()( Ogre::Root *root );

			Ogre::Vector3 vec;
	};
	*/


}	// namespace server

}	//namespace vl

#endif
