/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	TODO move to demos after new version from Windows is synced
 */

// Standard headers
#include <iostream>

// Library includes
#include "udp/server.hpp"
#include "udp/ogre_command.hpp"
#include "base/sleep.hpp"
#include "base/exceptions.hpp"
#include "eq_cluster/eq_settings.hpp"
#include "eq_cluster/nodeFactory.hpp"

// Test includes
#include "client_fixtures.hpp"

std::string const PROJECT_NAME( "udp_renderer" );
uint16_t const PORT = 2244;

/** The message is of form
  * Angle joint1
  * Angle joint2
  * Angle joint3
  * Position amr1
  * Position arm2
  */

class Channel : public eqOgre::Channel
{
public :
	Channel(eq::Window *parent)
		: eqOgre::Channel( parent ), server( PORT )
	{
		std::cout << "Starting UDP server on port " << PORT << std::endl;
		
		// TODO new message should be of form 3 x angle with predefined axis
		// and 3 x position
		// Find the ogre nodes
		std::vector< std::pair<std::string, Ogre::SceneNode *> > nodes;
		nodes.push_back( std::make_pair( "base", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "arm1", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "arm2", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "arm3", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "base_piston", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "base_rod", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "arm_piston", (Ogre::SceneNode *)0 ) );
		nodes.push_back( std::make_pair( "arm_rod", (Ogre::SceneNode *)0 ) );

		for( size_t i = 0; i < nodes.size(); ++i )
		{
			eqOgre::Window *win = (eqOgre::Window *)(getWindow());
			Ogre::SceneManager *sm = win->getSceneManager();
			std::string const &name = nodes.at(i).first;
			if( sm->hasSceneNode( name ) )
			{
				Ogre::SceneNode *node = sm->getSceneNode( name );
				nodes.at(i).second = node;
				// Add command to the server
				vl::udp::CommandRefPtr cmd = vl::udp::OgreCommand::create("setPosition", node );
				server.addCommand( cmd );
				cmd = vl::udp::OgreCommand::create("setAngleAxis", node );
				server.addCommand( cmd );
			}
			else
			{
				std::cerr << "Couldn't get SceneNode " << name << std::endl;
				throw vl::exception();
			}
		}
	}

	void frameDraw( const uint32_t frameID )
	{
		// Update UDP data
		try{
			server.mainloop();
		}
		catch( vl::long_message &e )
		{ std::cerr << "Exception : " << e.what() << std::endl; }

		// Draw the scene
		eqOgre::Channel::frameDraw( frameID );
	}

	vl::udp::Server server;
};

class NodeFactoryUDP : public ::NodeFactory
{
public :
	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }
};

int main(int argc, char **argv)
{
	bool error = false;
	try
	{
		ListeningClientFixture fix;

		eqOgre::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		eqOgre::NodeFactory nodeFactory;
		::NodeFactoryUDP nodeFactory;
		error = !fix.init( settings, &nodeFactory );

		if( !error )
		{
			uint32_t frame = 0;
			while( fix.mainloop(++frame) )
			{
				vl::msleep(1);
			}
		}
	}
	catch( vl::exception &e )
	{
		std::cerr << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		error = true;
	}
	catch( Ogre::Exception const &e)
	{
		std::cerr << "Ogre Exception: " << e.what() << std::endl;
		error = true;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
		error = true;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
