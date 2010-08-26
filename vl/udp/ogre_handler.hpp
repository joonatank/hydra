#ifndef OGREHANDLER_HPP
#define OGREHANDLER_HPP

#include "handler.hpp"
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
};

class OgreHandler : public udp::Handler
{
public:
	OgreHandler( int packet_id = -1 );

	virtual ~OgreHandler();

	virtual void handle( std::vector<double> msg );
};

}	// namespace udp

}	// namespace vl

#endif // OGREHANDLER_HPP
