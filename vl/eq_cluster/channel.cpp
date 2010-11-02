
#include "channel.hpp"

#include "window.hpp"
#include "config.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent ) 
	: eq::Channel(parent), _ogre_window(0), _viewport(0), _camera(0)
{}

eqOgre::Channel::~Channel( void )
{
}

bool
eqOgre::Channel::configInit( const uint32_t initID )
{
	if( !eq::Channel::configInit( initID ) )
	{ return false; }

	std::cerr << "Get ogre window from RenderWindow" << std::endl;
	eqOgre::Window *window = ((eqOgre::Window *)getWindow());
	_ogre_window = window->getRenderWindow();
	EQASSERT( _ogre_window );

	std::cerr << "Get camera from RenderWindow" << std::endl;
	_camera = window->getCamera();

	createViewport();

	// TODO this should be configurable from DotScene
	setNearFar( 0.1, 100.0 );

	// Get framedata
	eqOgre::Config *config = dynamic_cast< eqOgre::Config * >( getConfig() );
	if( !config )
	{
		EQERROR << "config is not type eqOgre::Config" << std::endl;
		return false;
	}

	uint32_t frame_id = config->getSettings()->getFrameDataID();
	EQASSERT( frame_id != EQ_ID_INVALID );
	_frame_data.mapData( config, frame_id );

	// We need to find the node from scene graph
	std::cerr << "FrameData has " << _frame_data.getNSceneNodes()
		<< " SceneNodes." << std::endl;
	Ogre::SceneManager *sm = window->getSceneManager();
	EQASSERT( sm );
	_frame_data.setSceneManager( sm );

	return true;
}

bool
eqOgre::Channel::configExit()
{
	_frame_data.unmapData( getConfig() );
	return eq::Channel::configExit();
}


/*
void
eqOgre::Channel::frameClear( const uint32_t )
{
//	TODO channel should do all rendering tasks
//	it should use Ogre::Viewport to do so.
//	if( _camera && _ogre_viewport )
//	{ _ogre_viewport->clear(); }
}
*/

/** Override frameDraw to call Viewport::update
 *
 *  Original does applyBuffer, applyViewport, applyFrustum, applyHeadTransform
 */
void
eqOgre::Channel::frameDraw( const uint32_t frameID )
{
	// Distribution
	_frame_data.syncAll();
	updateDistribData();

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

void 
eqOgre::Channel::setOgreFrustum( void )
{
	eq::Frustumf frust = getFrustum();
	_camera->setCustomProjectionMatrix( true, vl::math::convert( frust.compute_matrix() ) );

	Ogre::Matrix4 headMatrix = vl::math::convert( getHeadTransform() );
	Ogre::Vector3 cam_pos( _camera->getRealPosition() );//+ _frame_data.getHeadPosition() );
	// TODO Using the camera rotation is useful for Workstations, but it looks
	// really weird on VR systems. So it's ignored for now.
	// TODO Add a config value to control around which axises rotations
	// from camera are applied and which are not.
	// Y-axis should be fine for VR systems, X and Z are not.
	// FIXME
	// Y-axis is working fine other than that getPitch always returns the shortest
	// path so we can rotate the camera a bit but it returns back after that.
	Ogre::Quaternion cam_rot( _camera->getRealOrientation().getPitch(), Ogre::Vector3::UNIT_Y );
//	cam_rot = //Ogre::Quaternion::IDENTITY;
	Ogre::Matrix4 camViewMatrix = Ogre::Math::makeViewMatrix( cam_pos, cam_rot );
	// The multiplication order is correct (the problem is obvious if it's not)
	_camera->setCustomViewMatrix( true, headMatrix*camViewMatrix );
}

void 
eqOgre::Channel::updateDistribData( void )
{
	static uint32_t scene_version = 0;
	if( _frame_data.getSceneVersion() > scene_version )
	{
		// This will reload the scene but all transformations remain
		// As this will not reset the SceneNode structures that control the
		// transformations of objects.
		std::cerr << "Should reload the scene now" << std::endl;
		eqOgre::Window *win = static_cast<eqOgre::Window *>( getWindow() );
		win->loadScene();
		_camera = win->getCamera();
		createViewport();
		_frame_data.setSceneManager( win->getSceneManager() );
		
		scene_version = _frame_data.getSceneVersion();
	}
}

void 
eqOgre::Channel::createViewport( void )
{
	// Supports only one Viewport
	_ogre_window->removeAllViewports();
	std::cerr << "Creating viewport" << std::endl;
	_viewport = _ogre_window->addViewport( _camera );
	// TODO this should be configurable from DotScene
	_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
}
