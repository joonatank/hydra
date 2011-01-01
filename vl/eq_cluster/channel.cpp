/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

#include "channel.hpp"

#include "window.hpp"
#include "config.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent )
	: eq::Channel(parent), _viewport(0)
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

void
eqOgre::Channel::setViewport(Ogre::Viewport* viewport)
{
	_viewport = viewport;
}

bool
eqOgre::Channel::configInit( const eq::uint128_t &initID )
{
	if( !eq::Channel::configInit( initID ) )
	{ return false; }

	// TODO this should be configurable from DotScene
	setNearFar( 0.1, 100.0 );

	EQINFO << "Channel::ConfigInit done" << std::endl;

	return true;
}

bool
eqOgre::Channel::configExit()
{
	// Cleanup childs first
	bool retval = eq::Channel::configExit();

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
	// From equalizer channel::frameDraw
	// NOTE seems like we don't need these, Ogre should handle them anyway
	// TODO have to be tested on multiple walls and with head tracking though.
//	EQ_GL_CALL( applyBuffer( ));
//	EQ_GL_CALL( applyViewport( ));

// 	EQ_GL_CALL( glMatrixMode( GL_PROJECTION ) );
// 	EQ_GL_CALL( glLoadIdentity() );

	EQASSERT( _viewport );
	Ogre::Camera *camera = _viewport->getCamera();
	EQASSERT( camera );
	setOgreFrustum( camera );

	_viewport->update();
}

void
eqOgre::Channel::setOgreFrustum( Ogre::Camera *camera )
{
	EQASSERT( camera );
	eq::Frustumf frust = getFrustum();

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
