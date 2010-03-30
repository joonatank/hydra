/*	Joonatan Kuosa
 *	2010-03
 *
 * 	Pipe test
 *	Small test to determine if two pipes can render simultaneously.
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pipe_test

#include <boost/test/unit_test.hpp>

#include <eq/eq.h>

#include <fstream>

#include "eq_ogre/ogre_root.hpp"

#include "eq_test_fixture.hpp"

char *argv[argc] = { "PipeTest\0", "--eq-config\0", "2-pipe.eqc\0", "\0" };
//char *argv[argc] = { "PipeTest\0", "--eq-config\0", "1-window.eqc\0", "\0" };
//char *argv[argc] = { "pipes_test\0", "--eq-config\0", "2-nodes.eqc\0", "\0" };
//char *argv[argc] = { "/home/jotu/work/eqOgre/build/test/spikes/pipes_test\0", "\0" };

/*
class Node : public eq::Node
{
public :
	Node( eq::Config *config )
		: eq::Node( config ), root(0), win(0), state(-1)
	{
		//root = new vl::ogre::Root;

	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Node::configInit( initID ) );

		return true;
	}
	*/

//	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
//	{
		/*
		if( state < 0 )
		{
			std::cout << "Creating Windows" << std::endl;
			createWindows();
			++state;
		}
		else if( state < 1 )
		{
			std::cout << "Windowses created" << std::endl;
		}
		*/
//	}

//	void createWindows( void )
//	{
		/*
		for( size_t i = 0; i < getPipes().size(); i++ )
		{
			eq::Pipe *pipe = getPipes().at(i);
			std::cout << "Num windows = " << pipe->getWindows().size()
				<< std::endl;
			for( size_t j = 0; j < pipe->getWindows().size(); j++ )
			{
				eq::Window *window = pipe->getWindows().at(j);
				window->makeCurrent( false );
				std::stringstream name(
						std::stringstream::in | std::stringstream::out );
				name << "win-" << i << "-" << j;
				std::cout << "Creating window = " << name.str() << std::endl;
				vl::NamedValuePairList params;
				params["currentGLContext"] = std::string("True");
				
				//FIXME this breaks the loop for some reason.
				root->createWindow( name.str(), 800, 600, params );
			}
		}
		*/
		/*
		std::cout << win_desc_list.size() << " windows to create" << std::endl;
		Ogre::RenderWindowList win_list;
		BOOST_REQUIRE( root->getNative()
				->createRenderWindows( win_desc_list, win_list ));
		std::cout << win_list.size() << " windows created" << std::endl;
		*/
		//root->init();
		/*
	}

	vl::ogre::Root *root;
	vl::graph::RenderWindow *win;
	int state;
};
*/

class RenderWindow : public eq::Window
{
public :
	RenderWindow( eq::Pipe *parent )
		: eq::Window( parent ), root(0), win(0), cam(0), man(0), feet(0),
		robot(0)
	{
		boost::mutex::scoped_lock lock(test_mutex);

		root = new vl::ogre::Root;
		// Initialise ogre
		root->createRenderSystem();
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Window::configInit( initID ) );
		BOOST_REQUIRE( root );

		vl::NamedValuePairList params;
		params["currentGLContext"] = std::string("True");
		win = dynamic_cast<vl::ogre::RenderWindow *>(
				root->createWindow( "win", 800, 600, params ) );

		boost::mutex::scoped_lock lock(test_mutex);
		BOOST_REQUIRE( root );
		BOOST_CHECK_NO_THROW( man = root->createSceneManager("SceneManager") );
		BOOST_REQUIRE( man );
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << "Cam" << n_windows;
		BOOST_CHECK_NO_THROW( cam = man->createCamera(ss.str()) );
		BOOST_REQUIRE( win );
		vl::graph::Viewport *view;
		BOOST_CHECK_NO_THROW( view = win->addViewport( cam ) );
		BOOST_REQUIRE( view );
		BOOST_CHECK_NO_THROW( view->setBackgroundColour(
					vl::colour(1.0, 0.0, 0.0, 0.0) ) );

		BOOST_CHECK_NO_THROW( feet = man->getRootNode()->createChild("feet") );
		BOOST_CHECK_NO_THROW( feet->attachObject( cam ) );
		feet->lookAt( vl::vector(0,0,300) );

		vl::graph::Entity *ent;
		BOOST_CHECK_NO_THROW( ent = man->createEntity("robot", "robot.mesh") );
		ent->load(man);
		BOOST_CHECK_NO_THROW(
				robot = man->getRootNode()->createChild("robot") );
		BOOST_REQUIRE( robot );
		robot->setPosition( vl::vector(0, 0, 300) );
		BOOST_CHECK_NO_THROW( robot->attachObject( ent ) );
		
		++n_windows;

		return true;
	}
	
	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		boost::mutex::scoped_lock lock(test_mutex);

		win->update();
		win->swapBuffers();
	}

	static boost::mutex test_mutex;
	static int n_windows;

	vl::ogre::Root *root;
	vl::ogre::RenderWindow *win;
	vl::graph::Camera *cam;

	vl::graph::SceneManager *man;
	vl::graph::SceneNode *feet;
	vl::graph::SceneNode *robot;
};

int ::RenderWindow::n_windows = 0;
boost::mutex RenderWindow::test_mutex;
/*
class Pipe : public eq::Pipe
{
public :
	Pipe( eq::Node *parent )
		: eq::Pipe(parent), root(0), cam(0), man(0), feet(0), robot(0)
	{
		//root = new vl::ogre::Root;
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Pipe::configInit( initID ) );

		// Create Scene Manager
		//man = root->createSceneManager("SceneManager");
		//BOOST_REQUIRE( man );

		// create render windows
		//std::cout << "Num pipes = " << getPipes().size() << std::endl;
		//Ogre::RenderWindowDescriptionList win_desc_list;

		//eq::Window *window = getPipes().at(0)->getWindows().at(0);
		//BOOST_REQUIRE( window->isRunning() );
		//window->makeCurrent( false );

		return true;
	}

	vl::ogre::Root *root;
	vl::graph::Camera *cam;
	vl::graph::SceneManager *man;
	vl::graph::SceneNode *feet;
	vl::graph::SceneNode *robot;
};
*/

class NodeFactory : public eq::NodeFactory
{
public :
//	virtual eq::Pipe *createPipe( eq::Node *parent )
//	{ return new ::Pipe( parent ); }

	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new ::RenderWindow( parent ); }
};

eq::NodeFactory *g_nodeFactory = new ::NodeFactory;

BOOST_FIXTURE_TEST_CASE( render_test, EqFixture )
{
	BOOST_REQUIRE( config );

	for( size_t i = 0; i < 1000; i++ )
	{ mainloop(); }
}

