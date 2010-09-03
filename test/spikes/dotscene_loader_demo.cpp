#include "base/sleep.hpp"

#include "../fixtures.hpp"
#include "../debug.hpp"
#include <eq/client/channel.h>

#include "eq_ogre/ogre_root.hpp"
#include "settings.hpp"
#include "dotscene_loader.hpp"
#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "eq_cluster/window.hpp"
#include "fake_tracker.hpp"
#include "vrpn_tracker.hpp"

// Crashes channel.h for sure
// Seems like including vmmlib/vector.h or quaternion.h will crash channel (vmmlib/frustum.h)
#include "math/conversion.hpp"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>

// This class is different in dotscene without trackign and with tracking
// Should move this version to use FakeTracker so they would be more similar
class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), _ogre_window(0), _viewport(0), _camera(0),
		  _head_pos( Ogre::Vector3::ZERO ), _head_orient( Ogre::Quaternion::IDENTITY ), _tracker()
	{}

	virtual bool configInit( const uint32_t initID )
	{
		if( !eq::Channel::configInit( initID ) )
		{ return false; }

		std::cerr << "Get ogre window from RenderWindow" << std::endl;
		_ogre_window = ((eqOgre::Window *)getWindow())->getRenderWindow();
		EQASSERT( _ogre_window );

		std::cerr << "Get camera from RenderWindow" << std::endl;
		_camera = ((eqOgre::Window *)getWindow())->getCamera();
		EQASSERT( _camera );

		std::cerr << "Creating viewport" << std::endl;
		_viewport = _ogre_window->addViewport( _camera );
		_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );

		setNearFar( 0.1, 100.0 );

		_tracker = ((eqOgre::Window *)getWindow())->getTracker();
		EQASSERT( _tracker );

		return true;
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		setHeadMatrix();

		// From equalizer channel::frameDraw
		EQ_GL_CALL( applyBuffer( ));
	    EQ_GL_CALL( applyViewport( ));
	    
	    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ) );
	    EQ_GL_CALL( glLoadIdentity() );

		EQASSERT( _camera )
		EQASSERT( _ogre_window )
		
		setOgreFrustum();

		_viewport->update();
	}

	void setHeadMatrix( void )
	{
		// Head tracking support
		EQASSERT( _tracker );
			
		_tracker->mainloop();
		if( _tracker->getNSensors() > 0 )
		{
			_head_pos = _tracker->getPosition( 0 );
			_head_orient = _tracker->getOrientation( 0 );
		}

		Ogre::Matrix4 m( _head_orient ); 
		m.setTrans( _head_pos );

		// Note: real applications would use one tracking device per observer
	    const eq::Observers& observers = getConfig()->getObservers();
	    for( eq::Observers::const_iterator i = observers.begin();
			i != observers.end(); ++i )
	    {
			// When head matrix is set equalizer automatically applies it to the
			// GL Modelview matrix as first transformation
			(*i)->setHeadMatrix( vl::math::convert(m) );
		}
	}

	void setOgreFrustum( void )
	{
		eq::Frustumf frust = getFrustum();
		_camera->setCustomProjectionMatrix( true, vl::math::convert( frust.compute_matrix() ) );
		// TODO add support for cameras with parent
		// Ogre::Camera::getPosition returns position relative to the parent
		Ogre::Matrix4 viewMat = Ogre::Math::makeViewMatrix( _camera->getPosition() + _head_pos, _camera->getOrientation() ); //Ogre::Quaternion::IDENTITY );
		_camera->setCustomViewMatrix( true, viewMat );
	}

	Ogre::Camera *_camera;
	Ogre::Viewport *_viewport;
	Ogre::RenderWindow *_ogre_window;

	Ogre::Vector3 _head_pos;
	Ogre::Quaternion _head_orient;

	vl::TrackerRefPtr _tracker;
};

// This class is/should be the same in dotscene without trackign and with tracking
class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }

	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }
};

struct DotSceneFixture
{
	DotSceneFixture( void )
		: log_file( "render_test.log" ), config(0)
	{}

	~DotSceneFixture( void )
	{
		if( config )
		{ exit(); }
	}

	bool init( vl::SettingsRefPtr settings, eq::NodeFactory *nodeFactory )//, int argc, char** argv )
	{
		InitFixture();
		if( !settings )
		{
			std::cerr << "No settings provided";
			return -1;
		}

		vl::Args &arg = settings->getEqArgs();
		
		eq::base::Log::setOutput( log_file );
		
		// 1. Equalizer initialization
		if( !eq::init( arg.size(), arg.getData(), nodeFactory ))
		{
			EQERROR << "Equalizer init failed" << std::endl;
			return false;
		}

		// 2. get a configuration
		config = static_cast< eqOgre::Config * >( eq::getConfig( arg.size(), arg.getData() ) );
		if( config )
		{
			config->setSettings( settings );
			// 3. init config
			if( !config->init(0) )
			{
				EQERROR << "Config initialization failed: "
						<< config->getErrorMessage() << std::endl;
				return false;
			}
		}
		else
		{
			EQERROR << "Cannot get config" << std::endl;
			return false;
		}

		return true;
	}

	bool mainloop( const uint32_t frame )
	{
		// 4. run main loop
		if( config->isRunning() )
		{
			config->startFrame( frame );
			config->finishFrame();
			return true;
		}
		else
		{ return false; }
	}

	void exit( void )
	{
		// 5. exit config
		config->exit();

		// 6. release config
		eq::releaseConfig( config );

		config = 0;
		
		// 7. exit
		eq::exit();
	}

	std::ofstream log_file;

	eqOgre::Config *config;
};

// This function is the same in dotscene without trackign and with tracking
int main( const int argc, char** argv )
{
	bool error = false;
	try
	{
		DotSceneFixture fix;

		vl::SettingsRefPtr settings = getSettings(argv[0]);
		if( !settings )
		{
			std::cerr << "No test_conf.xml file found." << std::endl;
			return -1;
		}

		::NodeFactory nodeFactory;

		error = !fix.init( settings, &nodeFactory );//, argc, argv );
	
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
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
