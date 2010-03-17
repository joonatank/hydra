/*	Joonatan Kuosa
 *	2010-03
 *
 *	Small test to test rendering with Ogre using the equalizer
 *	rendering loop.
 *	Very small equalizer and Ogre initialization.
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ogre_entity

#include <eq/eq.h>

#include <fstream>

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"

class Node : public eq::Node
{
public :
	Node( eq::Config *parent )
		: eq::Node(parent), state(0), ogre_root(0), man(0), feet(0), robot(0)
	{} 

	virtual ~Node( void )
	{
		delete ogre_root;
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Node::configInit( initID ) );

		ogre_root = new vl::ogre::Root();
		man = ogre_root->createSceneManager("SceneManager");
		BOOST_REQUIRE( man );
		vl::graph::SceneNode *root = man->getRootNode();
		BOOST_REQUIRE( root );
		feet = root->createChild();
		robot = root->createChild();
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Node::frameStart( frameID, frameNumber );

		switch( state )
		{
			case 0 :
			{
				++state;
			}
			break;

			case 1 :
			{
				++state;

			}
			break;

			case 2 :
			{
				++state;
			}
			break;
			case 3 :
			{
				++state;
			}
			break;

			case 4 :
			{
				++state;
			}
			break;

			case 5 :
			{ 
				++state;
			}
			break;

			default :
			break;
		}

	}

	vl::graph::Root *ogre_root;
	vl::graph::SceneManager *man;
	vl::graph::SceneNode *feet;
	vl::graph::SceneNode *robot;
	int state;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Node *createNode( eq::Config *parent )
	{ return new ::Node( parent ); }
};

const int argc = 2;
char NAME[] = "TEST\0";
char *argv[argc] = { NAME, "\0" };

struct RenderFixture
{
	// Init code for this test
	RenderFixture( void )
		: error( false ), frameNumber(0), config(0),
		  log_file( "render_test.log" )
	{
		// Redirect logging
		eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, &nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		// 3. init config
		BOOST_REQUIRE( config->init(0));
	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );
	
		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~RenderFixture( void )
	{
		// 5. exit config
		if( config )
		{ BOOST_CHECK( config->exit() ); }

		// 6. release config
		eq::releaseConfig( config );

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	bool error;
	uint32_t frameNumber;
	eq::Config *config;
	NodeFactory nodeFactory;
	std::ofstream log_file;
};

BOOST_FIXTURE_TEST_CASE( render_test, RenderFixture )
{
	BOOST_REQUIRE( config );

	for( size_t i = 0; i < 1000; i++ )
	{ mainloop(); }
}
