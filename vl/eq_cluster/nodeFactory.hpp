/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */
#ifndef EQ_OGRE_NODEFACTORY_HPP
#define EQ_OGRE_NODEFACTORY_HPP

#include <eq/client/nodeFactory.h>
#include <eq/client/pipe.h>
#include <eq/client/node.h>

#include "config.hpp"
#include "channel.hpp"
#include "window.hpp"
#include "pipe.hpp"

namespace eqOgre
{

class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }

	virtual eq::Pipe *createPipe( eq::Node *parent )
	{ return new eqOgre::Pipe( parent ); }

	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new eqOgre::Channel( parent ); }

};

}

#endif
