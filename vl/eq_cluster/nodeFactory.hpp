#ifndef EQ_OGRE_NODEFACTORY_HPP
#define EQ_OGRE_NODEFACTORY_HPP

#include <eq/client/nodeFactory.h>
#include <eq/client/pipe.h>
#include <eq/client/node.h>

#include "config.hpp"
#include "channel.hpp"
#include "window.hpp"

namespace eqOgre
{

class Pipe : public eq::Pipe
{
public :
	Pipe( eq::Node* parent )
		: eq::Pipe( parent )
	{}
protected :
	virtual void frameFinish( const eq::uint128_t &frameID, uint32_t const frameNumber  )
	{
		eq::Pipe::frameFinish( frameID, frameNumber );
		std::cerr << "eq::Pipe::frameFinish" << std::endl;
	}

	virtual void frameDrawFinish( const eq::uint128_t &frameID, uint32_t const frameNumber   )
	{
		std::cerr << "eq::Pipe::frameDrawFinish" << std::endl;
		eq::Pipe::frameDrawFinish( frameID, frameNumber );
		std::cerr << "eq::Pipe::frameDrawFinish done" << std::endl;
	}

};

class Node : public eq::Node
{
public :
	Node( eq::Config* parent )
		: eq::Node( parent )
	{}

protected :
	virtual void frameStart( const eq::uint128_t &frameID, uint32_t const frameNumber   )
	{
		std::cerr << "eq::Node::frameStart" << std::endl;
		eq::Node::frameStart( frameID, frameNumber );
		std::cerr << "eq::Node::frameStart done" << std::endl;
	}

	virtual void frameDrawFinish( const eq::uint128_t &frameID, uint32_t const frameNumber   )
	{
		std::cerr << "eq::Node::frameDrawFinish" << std::endl;
		eq::Node::frameDrawFinish( frameID, frameNumber );
		std::cerr << "eq::Node::frameDrawFinish done" << std::endl;
	}

	virtual void frameFinish( const eq::uint128_t &frameID, uint32_t const frameNumber  )
	{
		std::cerr << "eq::Node::frameFinish" << std::endl;
		eq::Node::frameFinish( frameID, frameNumber );
		std::cerr << "eq::Node::frameFinish done" << std::endl;
	}
};

class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }

	virtual eq::Node* createNode( eq::Config* parent )
	{ return new eqOgre::Node( parent ); }

	virtual eq::Pipe* createPipe( eq::Node* parent )
	{ return new eqOgre::Pipe( parent ); }

	virtual eq::Window* createWindow( eq::Pipe* parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Channel *createChannel( eq::Window* parent )
	{ return new eqOgre::Channel( parent ); }

};

}

#endif
