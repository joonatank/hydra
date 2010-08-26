#ifndef VL_UDP_OGRE_COMMAND_HPP
#define VL_UDP_OGRE_COMMAND_HPP

#include "command.hpp"
#include <OGRE/OgreSceneNode.h>

namespace vl
{
	
namespace udp
{

class OgreCommand : public vl::udp::Command
{
public :
	OgreCommand( std::string const &typ, Ogre::SceneNode *node );
	
	/// Execute the command
	virtual void operator()( void );

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual uint16_t getSize( void ) const;

	virtual CMD_TYPE getType( void ) const
	{ return _type; }

	virtual double &at( size_t i )
	{ return _data.at(i); }

	virtual double const &at( size_t i ) const
	{ return _data.at(i); }

private :
	Ogre::SceneNode *_node;
	std::vector<double> _data;
	CMD_TYPE _type;

};	// class OgreCommand

}	// namespace udp

}	// namespace vl

#endif // VL_UDP_OGRE_COMMAND_HPP
