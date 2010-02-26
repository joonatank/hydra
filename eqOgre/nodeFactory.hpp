#ifndef EQ_OGRE_NODEFACTORY_HPP
#define EQ_OGRE_NODEFACTORY_HPP

#include <eq/client/nodeFactory.h>

#include "config.hpp"
#include "node.hpp"
#include "pipe.hpp"
#include "channel.hpp"
#include "window.hpp"

namespace eqOgre {

class NodeFactory : public eq::NodeFactory
{
	public:
//		eqOgre::NodeFactory &getInstance( void );
//		eqOgre::NodeFactory *getInstancePtr( void );
		
		virtual eq::Config *createConfig( eq::ServerPtr parent )
		{ return new eqOgre::Config( parent ); }

		virtual eq::Node *createNode( eq::Config* parent )  
		{ return new eqOgre::Node( parent ); }

		virtual eq::Pipe *createPipe( eq::Node* parent )
		{ return new eqOgre::Pipe( parent ); }

		virtual eq::Window* createWindow( eq::Pipe* parent )
		{ return new eqOgre::Window( parent ); }

		virtual eq::Channel *createChannel( eq::Window* parent )
		{ return new eqOgre::Channel( parent ); }

//		virtual eq::View *createView( void )
//		{ return new eqOgre::View(); }
	
//	private :
//		static eqOgre::NodeFactory *_instance;
};

}

#endif
