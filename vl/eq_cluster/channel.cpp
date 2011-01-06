/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *	2011-01 Updated : 
 *	Removed the use of Equalizer Wall.
 *	Removed the use of Equalizer headTracker position.
 *	Using our own distributed player object for head position.
 *	Added custom projection and view matrix creation.
 *	Wall definition in Environment configuration file, mapped to eq chanel by name.
 *	Frustum is created correctly now for different projection walls.
 *	Frustum is not moved anymore to z direction with head tracker.
 *	Added stereo rendering code. Tested without quad-buffer, seems to work.
 *	TODO test with quad-buffer stereo.
 *
 */

#include "channel.hpp"

#include "window.hpp"
#include "config.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent )
	: eq::Channel(parent), _viewport(0), _stereo(false)
{}

eqOgre::Channel::~Channel( void )
{}

eqOgre::DistributedSettings const &
eqOgre::Channel::getSettings( void ) const
{
	EQASSERT( dynamic_cast<eqOgre::Window const *>( getWindow() ) );
	return static_cast<eqOgre::Window const *>( getWindow() )->getSettings();
}

vl::Player const &
eqOgre::Channel::getPlayer( void ) const
{
	EQASSERT( dynamic_cast<eqOgre::Window const *>( getWindow() ) );
	return static_cast<eqOgre::Window const *>( getWindow() )->getPlayer();
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

	EQASSERT( getSettings().getNWalls() > 0);
	EQINFO << "Settings has " << getSettings().getNWalls() << " wall configs." 
		<< std::endl;

	// If the channel has a name we try to find matching wall
	if( !getName().empty() )
	{
		EQINFO << "Finding Wall for channel : " << getName() << std::endl;
		_wall = getSettings().findWall( getName() );
	}
	
	// Get the first wall definition if no named one was found
	if( _wall.empty() )
	{
		_wall = getSettings().getWall(0);
	}
	
	EQASSERT( !_wall.empty() );

	EQINFO << "Using wall : " << _wall.name <<std::endl; 

	std::cout << "Checking for stereo." << std::endl;
	GLboolean stereo;
	glGetBooleanv( GL_STEREO, &stereo );
	_stereo = stereo;
	if( _stereo )
	{
		std::cout << "Stereo supported" << std::endl;
	}
	else
	{
		std::cout << "No stereo support." << std::endl;
	}

	std::cout << "IPD = " << getSettings().getIPD() << std::endl;

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
	EQASSERT( _viewport );
	Ogre::Camera *camera = _viewport->getCamera();
	EQASSERT( camera );

	// TODO test the real stereo rendering code
	if( _stereo )
	{
		double ipd = getSettings().getIPD();

		//draw into back left buffer
		glDrawBuffer(GL_BACK_LEFT);                    
		Ogre::Vector3 eye(-ipd/2, 0, 0);
		setOgreFrustum( camera, eye );
		setOgreView( camera, eye );
		_viewport->update();
		
		//draw into back right buffer
		glDrawBuffer(GL_BACK_RIGHT);                             
		eye = Ogre::Vector3(ipd/2, 0, 0);
		setOgreFrustum( camera, eye );
		setOgreView( camera, eye );
		_viewport->update();
	}
	else
	{
		setOgreFrustum( camera );
		setOgreView( camera );
		_viewport->update();
	}
}

void
eqOgre::Channel::setOgreFrustum( Ogre::Camera *camera, Ogre::Vector3 eye )
{
	EQASSERT( camera );
	EQASSERT( !_wall.empty() );

	/* Projection matrix i.e. frustum
	 * | E	0	A	0 |
	 * | 0	F	B	0 |
	 * | 0	0	C	D |
	 * | 0	0	-1	0 |
	 *
	 * where 
	 * A = -(right + left)/(right - left)
	 * B = -(top + bottom)/(top - bottom)
	 * C = -(far + near )/(far - near )
	 * D = -2*far*near/(far - near)
	 * E = 2*near/(right - left)
	 * F = 2*near/(top - bottom)
	 */
	
	Ogre::Real c_near = camera->getNearClipDistance();
	Ogre::Real c_far = camera->getFarClipDistance();

	Ogre::Matrix4 const &headMat = getPlayer().getHeadMatrix();

	Ogre::Vector3 bottom_right( _wall.bottom_right.at(0), _wall.bottom_right.at(1), _wall.bottom_right.at(2) );
	Ogre::Vector3 bottom_left( _wall.bottom_left.at(0), _wall.bottom_left.at(1), _wall.bottom_left.at(2) );
	Ogre::Vector3 top_left( _wall.top_left.at(0), _wall.top_left.at(1), _wall.top_left.at(2) );

	Ogre::Plane plane(bottom_right, bottom_left, top_left);

	Ogre::Vector3 cam_vec(-Ogre::Vector3::UNIT_Z);
	Ogre::Vector3 plane_normal = plane.normal.normalisedCopy();
	Ogre::Quaternion wallRot = plane_normal.getRotationTo(cam_vec);
	Ogre::Vector3 headTrans = wallRot*headMat.getTrans();

	bottom_right = wallRot*bottom_right;
	bottom_left = wallRot*bottom_left;
	top_left = wallRot*top_left;

	// Calculate the frustum
	Ogre::Real wall_right = bottom_right.x;
	Ogre::Real wall_left = bottom_left.x;
	Ogre::Real wall_top = top_left.y;
	Ogre::Real wall_bottom = bottom_right.y;
	Ogre::Real wall_front = bottom_right.z;

	// eye is relative to the head, and has the same orientation
	// eye orientation should modify the frustum when rotated around y and z
	// but should stay constant when rotated around x.
	// Tested it works that way.
	Ogre::Quaternion headQuat = headMat.extractQuaternion();
	eye = headQuat*eye + headTrans;

	// The coordinates right, left, top, bottom
	// represent a view frustum with coordinates (left, bottom, -near)
	// and (right, top, -near)
	// So the wall and head needs to be scaled by the z-coordinate to
	// obtain the correct scale
	// If scale is negative it rotates 180 deg around z, 
	// i.e. flips to the other side of the wall
	// Scale can not include the head_z because the frustum gets screwed up if
	// it does. Might need to scale it a bit before using it or something.
	// TODO have a look at what it is supposed to be.
	//Ogre::Real scale = (wall_front+head_z)/(-c_near);
	Ogre::Real scale = (wall_front)/(-c_near);
	Ogre::Real right = (wall_right - eye.x)/scale;
	Ogre::Real left = (wall_left - eye.x)/scale;
	Ogre::Real top = (wall_top - eye.y)/scale;
	Ogre::Real bottom = (wall_bottom - eye.y)/scale;

	// Near and far clipping should not be modified because
	// Increasing the near clip would clip the objects near the user.
	// Decreasing the near clip would go to negative, which is not allowed.
	Ogre::Real A = (right + left)/(right - left);
	Ogre::Real B = (top + bottom)/(top - bottom);
	Ogre::Real C = -(c_far + c_near )/(c_far - c_near );
	Ogre::Real D = -2*c_far*c_near/(c_far - c_near);
	Ogre::Real E = 2*c_near/(right - left);
	Ogre::Real F = 2*c_near/(top - bottom);

	Ogre::Matrix4 projMat;
	projMat[0][0] = E;
	projMat[0][1] = 0;
	projMat[0][2] = A;
	projMat[0][3] = 0;

	projMat[1][0] = 0;
	projMat[1][1] = F;
	projMat[1][2] = B;
	projMat[1][3] = 0;

	projMat[2][0] = 0;
	projMat[2][1] = 0;
	projMat[2][2] = C;
	projMat[2][3] = D;
	
	projMat[3][0] = 0;
	projMat[3][1] = 0;
	projMat[3][2] = -1;
	projMat[3][3] = 0;

	camera->setCustomProjectionMatrix( true, projMat );
}

void
eqOgre::Channel::setOgreView( Ogre::Camera *camera, Ogre::Vector3 eye )
{
	EQASSERT( camera );

	// Get modified camera orientation
	Ogre::Quaternion cam_rot( camera->getRealOrientation() );
	Ogre::Quaternion cam_orient;

	// Y-axis should be fine for VR systems, X and Z are not.
	// NOTE if these rotations are disabled when moving the camera node
	// the moving direction is different than the direction the camera is facing
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

	Ogre::Matrix4 const &headMat = getPlayer().getHeadMatrix();
	// NOTE This is not HMD discard the rotation part
	// This doesn't seem to have any affect. 
	// Though it's more realistic if it's there.
	eye = headMat.extractQuaternion()*eye + headMat.getTrans();
	Ogre::Vector3 cam_pos( camera->getRealPosition() );

	// TODO wall transformation should transform the camera
	// e.g. left wall should rotate the camera by 90 deg to left
	// we need to calculate the middle coordinate for the wall/plane
	// and take it's transformation take the rotational part and apply it to
	// the camera orientation before creating the view matrix

	
	//Ogre::Vector3 head_pos = headMat.getTrans();
	Ogre::Matrix4 camViewMatrix = Ogre::Math::makeViewMatrix( cam_pos+eye, cam_orient );

	camera->setCustomViewMatrix( true, camViewMatrix );
}
