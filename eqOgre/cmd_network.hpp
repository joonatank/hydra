/*	Joonatan Kuosa 2010-01
 *	This provides RakNet methods for transmitting commands over network
 *	because this is not integral part of the command system we define those
 *	here rather than in command.hpp. (also these need RakNet headers).
 */

#ifndef EQ_OGRE_CMD_NETWORK_HPP
#define EQ_OGRE_CMD_NETWORK_HPP

#include "command.hpp"

#include "typedefs.hpp"

// For math
#include <vmmlib/vector.hpp>
#include <vmmlib/quaternion.hpp>

#include <raknet/BitStream.h>
#include <raknet/RakString.h>
#include <raknet/MessageIdentifiers.h>

// TODO move this to separate header, so we have all packet ids in same place
enum {
	ID_VL_COMMAND = ID_USER_PACKET_ENUM
};

namespace vl 
{
	// For now in namespace base... should be in vl::net
	namespace base
	{
	
	inline RakNet::BitStream &
	operator<<( RakNet::BitStream& out, vmml::vec3d const &v )
	{
		out.Write(v(0));
		out.Write(v(1));
		out.Write(v(2));

		return out;
	}

	inline RakNet::BitStream &
	operator>>( RakNet::BitStream& in, vmml::vec3d &v )
	{
		in.Read(v(0));
		in.Read(v(1));
		in.Read(v(2));

		return in;
	}

	inline RakNet::BitStream &
	operator<<( RakNet::BitStream& out, vmml::quaterniond const &q )
	{
		out.Write(q(0));
		out.Write(q(1));
		out.Write(q(2));
		out.Write(q(3));

		return out;
	}

	inline RakNet::BitStream &
	operator>>( RakNet::BitStream& in, vmml::quaterniond& q )
	{
		in.Read(q(0));
		in.Read(q(1));
		in.Read(q(2));
		in.Read(q(3));

		return in;
	}

	inline RakNet::BitStream &
	operator<<( RakNet::BitStream& out, std::string const &str )
	{
		out.Write( RakNet::RakString( str.c_str() ) );

		return out;
	}

	inline RakNet::BitStream &
	operator>>( RakNet::BitStream& in, std::string &str )
	{
		RakNet::RakString rak_str;
		in.Read( rak_str );
		str = rak_str.C_String();

		return in;
	}

	inline RakNet::BitStream &
	operator<<( RakNet::BitStream& out, vl::NamedValuePairList const &pairlist )
	{
		out.Write( pairlist.size() );
		vl::NamedValuePairList::const_iterator iter;
		for( iter = pairlist.begin(); iter != pairlist.end(); ++iter )
		{
			out.Write( RakNet::RakString( iter->first.c_str() ) );
			out.Write( RakNet::RakString( iter->second.c_str() ) );
		}
		return out;
	}

	inline RakNet::BitStream &
	operator>>( RakNet::BitStream& in, vl::NamedValuePairList &pairlist )
	{
		size_t size = 0;
		in.Read( size );
		pairlist.clear();
		for( size_t i = 0; i < size; i++ );
		{
			RakNet::RakString first, second;
			in.Read( first );
			in.Read( second );

			pairlist[Ogre::String( first )] = Ogre::String( second);
		}

		return in;
	}


	inline RakNet::BitStream &
	operator<<( RakNet::BitStream& out, vl::base::CommandStruct const &cmd )
	{
		if( cmd.cmdType == CMD_INVALID )
		{ return out; }

		out.Write( (unsigned char)ID_VL_COMMAND );
		out.Write( cmd.cmdType );
		out << cmd.name << cmd.typeName << cmd.creator;
		out << cmd.parent;
		out << cmd.position;
		out << cmd.rotation;
		out << cmd.space;
		out << cmd.params;

		return out;
	}

	inline RakNet::BitStream &
	operator>>( RakNet::BitStream& in, vl::base::CommandStruct &cmd )
	{
		in.Read( cmd.cmdType );
		in >> cmd.name >> cmd.typeName >> cmd.creator >> cmd.parent
			>> cmd.position >> cmd.rotation >> cmd.space >> cmd.params;

		return in;
	}

	/*
	inline RakNet::BitStream &
	operator<<( RakNet::BitStream& out, vl::base::Command const &cmd )
	{
		if( cmd.cmdType == CMD_INVALID )
		{ return out; }

		out.Write( (unsigned char)ID_VL_COMMAND );
		out.Write( cmd.cmdType );
		out << cmd.name << cmd.typeName << cmd.creator;

		switch( cmd.cmdType )
		{
			case CMD_INVALID:
				break;
			case CMD_CREATE:
				{
					vl::base::CreateCmd &cc = (vl::base::CreateCmd &)cmd;
					out << cc.parent;
					out << cc.params;
				}
				break;
			case CMD_DELETE:
				// No extra parameters
				break;
			case CMD_MOVE:
				{
					vl::base::MoveCmd &cc = (vl::base::MoveCmd &)cmd;
					out << cc.vec;
				}
				break;

			case CMD_ROTATE:
				{
					vl::base::RotateCmd &cc = (vl::base::RotateCmd &)cmd;
					out << cc.quat;
				}
				break;
			case CMD_LOOKAT:
				{
					vl::base::LookAtCmd &cc = (vl::base::LookAtCmd &)cmd;
					out << cc.vec;
				}
				break;

			// TODO throw error as these should never be sent over network
			case CMD_ROOT_TRANS:
			case CMD_CAMERA_TRANS:
			case CMD_WINDOW_TRANS:
			default:
				break;
		}

		return out;
	}

	// Using auto_ptrs for Commands atm.
	// Reason we can do allocation/deallocation here and the save the result
	// into the auto_ptr.
	// TODO replace auto_ptr with something simpler if necessary
	// it's only responsibility is to be static place holder for Command
	// which type is not know before we read from BitStream.
	inline RakNet::BitStream &
	operator>>( RakNet::BitStream& in, std::auto_ptr<vl::base::Command> &cmd )
	{
		unsigned int type = CMD_INVALID;
		in.Read( type );

		// Should throw
		if( type == CMD_INVALID )
		{ return in; }

		Ogre::String name, typeName, creator;
		in >> name >> typeName >> creator;

		switch( type )
		{
			case CMD_CREATE:
			{
				Ogre::String parent;
				Ogre::NameValuePairList params;
				in >> parent;
				in >> params;
				cmd.reset( new vl::base::CreateCmd( name, typeName, parent,
						params, creator ) );
			}
				break;
			case CMD_DELETE:
				// No extra parameters
					cmd.reset( new vl::base::DeleteCmd( name, typeName, creator ) );
				break;
			case CMD_MOVE:
			{
				Ogre::Vector3 vec;
				in >> vec;
				cmd.reset( new vl::base::MoveCmd( name, typeName, vec, creator ) );
			}
				break;

			case CMD_ROTATE:
			{
				Ogre::Quaternion q;
				in >> q;
				cmd.reset( new vl::base::RotateCmd( name, typeName, q, creator ) ); 
			}
				break;
			case CMD_LOOKAT:
			{
				Ogre::Vector3 vec;
				in >> vec;
				cmd.reset( new vl::base::LookAtCmd( name, typeName, vec, creator ) );
			}
				break;

			// TODO throw error as these should never be sent over network
			case CMD_ROOT_TRANS:
			case CMD_CAMERA_TRANS:
			case CMD_WINDOW_TRANS:
				break;
		}

		return in;
	}
		*/

	}	// namespace base 

};	// namespace vl

#endif
