
#include "channel.hpp"

#include "window.hpp"
#include "config.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent )
	: eq::Channel(parent), _ogre_window(0), _viewport(0)
{}

eqOgre::Channel::~Channel( void )
{}

eqOgre::DistributedSettings const &
eqOgre::Channel::getSettings( void ) const
{
	EQASSERT( dynamic_cast<eqOgre::Window const *>( getWindow() ) );
	return static_cast<eqOgre::Window const *>( getWindow() )->getSettings();
}

void
eqOgre::Channel::setCamera( Ogre::Camera *cam )
{
	if( _viewport )
	{ _viewport->setCamera(cam); }
}

bool
eqOgre::Channel::configInit( const eq::uint128_t &initID )
{
	if( !eq::Channel::configInit( initID ) )
	{ return false; }

	EQINFO << "Get ogre window from RenderWindow" << std::endl;
	eqOgre::Window *window = dynamic_cast<eqOgre::Window *>(getWindow());
	_ogre_window = window->getRenderWindow();
	if( !_ogre_window )
	{ return false; }

	Ogre::Camera *camera = window->getCamera();

	createViewport( camera );

	// TODO this should be configurable from DotScene
	setNearFar( 0.1, 100.0 );

	// Get framedata
	eqOgre::Config *config = dynamic_cast< eqOgre::Config * >( getConfig() );
	if( !config )
	{
		EQERROR << "Config is not type eqOgre::Config" << std::endl;
		return false;
	}

	eq::base::UUID const &frame_id = getSettings().getSceneManagerID();
	EQASSERT( frame_id != eq::base::UUID::ZERO );
	_frame_data.mapData( config, frame_id );

	// We need to find the node from scene graph
	EQINFO << "FrameData has " << _frame_data.getNSceneNodes()
		<< " SceneNodes." << std::endl;

	Ogre::SceneManager *sm = window->getSceneManager();
	EQASSERTINFO( sm, "Window has no Ogre SceneManager" );
	if( !_frame_data.setSceneManager( sm ) )
	{
		EQERROR << "Some SceneNodes were not found." << std::endl;
	}

	EQINFO << "Channel::ConfigInit done" << std::endl;

	return true;
}

bool
eqOgre::Channel::configExit()
{
	// Cleanup childs first
	bool retval = eq::Channel::configExit();

	// Unmap data
	EQINFO << "Unmapping SceneManager." << std::endl;
	_frame_data.unmapData();

	return retval;
}


// NOTE overload with empty function
// seems like we don't need these, Ogre Viewport will clear it self
// before rendering if we don't instruct it to do otherwise
/*
void
eqOgre::Channel::frameClear( const eq::uint128_t & )
{
//	TODO channel should do all rendering tasks
//	it should use Ogre::Viewport to do so.
	if( _camera && _ogre_viewport )
	{ _ogre_viewport->clear(); }
}
*/

/** Override frameDraw to call Viewport::update
 *
 *  Original does applyBuffer, applyViewport, applyFrustum, applyHeadTransform
 */
void
eqOgre::Channel::frameDraw( const eq::uint128_t &frameID )
{
	// Distribution
	updateDistribData();

	// From equalizer channel::frameDraw
	// NOTE seems like we don't need these, Ogre should handle them anyway
	// TODO have to be tested on multiple walls and with head tracking though.
//	EQ_GL_CALL( applyBuffer( ));
//	EQ_GL_CALL( applyViewport( ));

// 	EQ_GL_CALL( glMatrixMode( GL_PROJECTION ) );
// 	EQ_GL_CALL( glLoadIdentity() );

	EQASSERT( _viewport )

	setOgreFrustum();

	_viewport->update();
}

void
eqOgre::Channel::setOgreFrustum( void )
{
	eq::Frustumf frust = getFrustum();
	Ogre::Camera *camera = _viewport->getCamera();
	camera->setCustomProjectionMatrix( true, vl::math::convert( frust.compute_matrix() ) );

	Ogre::Matrix4 headMatrix = vl::math::convert( getHeadTransform() );
	Ogre::Vector3 cam_pos( camera->getRealPosition() );

	// TODO Add a config value to control around which axises rotations
	// from camera are applied and which are not.
	// Y-axis should be fine for VR systems, X and Z are not.
	// NOTE if these rotations are disabled when moving the camera node
	// the moving direction is different than the direction the camera is facing

	// Get modified camera orientation
	Ogre::Quaternion cam_rot( camera->getRealOrientation() );
	Ogre::Quaternion cam_orient;
	uint32_t cam_rot_flags = getSettings().getCameraRotationAllowed();
	if( cam_rot_flags == (1 | 1<<1 | 1<<2 ) )
	{
		cam_orient = cam_rot;
	}
	else
	{
		Ogre::Radian x, y, z;
		vl::getEulerAngles( cam_rot, x, y, z );
		Ogre::Quaternion q_x, q_y, q_z;
		if( !(cam_rot_flags & 1) )
			x = Ogre::Radian(0);
		if( !(cam_rot_flags & (1<<1) ) )
			y = Ogre::Radian(0);
		if( !(cam_rot_flags & (1<<2) ) )
			z = Ogre::Radian(0);

		vl::fromEulerAngles( cam_orient, x, y, z );
	}

	Ogre::Matrix4 camViewMatrix = Ogre::Math::makeViewMatrix( cam_pos, cam_orient );
	// The multiplication order is correct (the problem is obvious if it's not)
	camera->setCustomViewMatrix( true, headMatrix*camViewMatrix );
}

void
eqOgre::Channel::updateDistribData( void )
{
	// Update SceneManager
	// TODO should be moved to Window or even better to Pipe
	_frame_data.syncAll();
	static uint32_t scene_version = 0;
	if( _frame_data.getSceneVersion() > scene_version )
	{
		// This will reload the scene but all transformations remain
		// As this will not reset the SceneNode structures that control the
		// transformations of objects.
		EQINFO << "Reloading the Ogre scene now" << std::endl;
		eqOgre::Window *win = static_cast<eqOgre::Window *>( getWindow() );
		win->loadScene();
		Ogre::Camera *camera = win->getCamera();
		createViewport( camera );
		_frame_data.setSceneManager( win->getSceneManager() );
		EQINFO << "Ogre Scene reloaded." << std::endl;

		scene_version = _frame_data.getSceneVersion();
	}
}

void
eqOgre::Channel::createViewport( Ogre::Camera *cam )
{
	EQINFO << "Creating viewport" << std::endl;
	EQASSERT( cam );
	// Supports only one Viewport
	_ogre_window->removeAllViewports();

	_viewport = _ogre_window->addViewport( cam );
	// TODO this should be configurable from DotScene
	_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
	_viewport->setAutoUpdated(false);
}
