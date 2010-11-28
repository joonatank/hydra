#ifndef EQ_OGRE_NODEFACTORY_HPP
#define EQ_OGRE_NODEFACTORY_HPP

#include <eq/client/nodeFactory.h>

#include "config.hpp"
#include "channel.hpp"
#include "window.hpp"

namespace eqOgre
{

class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }

	virtual eq::Window* createWindow( eq::Pipe* parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Channel *createChannel( eq::Window* parent )
	{ return new eqOgre::Channel( parent ); }

};

}

#endif
