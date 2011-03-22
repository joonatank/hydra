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
 *	Added stereo rendering code. Tested with quad-buffer.
 *	Added rotation of the view matrix based on projection plane.
 *	Fixed scaling the frustum based on head tracker.
 *		Would create non-continiuty on front and side walls otherwise.
 */

#include "channel.hpp"

#include "window.hpp"
#include "config.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"
#include "base/string_utils.hpp"

#include <OGRE/OgreLogManager.h>

#include <GL/gl.h>
#include "pipe.hpp"

vl::Channel::Channel( Ogre::Viewport *viewport,
					  vl::EnvSettings::Channel const &chanConf,
					  vl::EnvSettings::Wall const &wall, double ipd )
	: _channel_conf(chanConf)
	, _wall(wall)
	, _stereo(false)
	, _ipd(ipd)
	, _viewport(viewport)
	, _camera(0)
{
	assert( !_wall.empty() );
	assert( _viewport );

	GLboolean stereo;
	glGetBooleanv( GL_STEREO, &stereo );
	_stereo = stereo;
	if( _stereo )
	{
		std::string msg = "Stereo supported";
		Ogre::LogManager::getSingleton().logMessage(msg);
		msg = "IPD (Inter pupilar distance) = " + vl::to_string(_ipd);
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
	else
	{
		std::string msg = "No stereo support.";
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
}

vl::Channel::~Channel( void )
{}

void
vl::Channel::setCamera( Ogre::Camera *cam )
{
	_viewport->setCamera(cam);
	_camera = cam;
}


void
vl::Channel::draw( void )
{
	assert( _camera );
	assert( _viewport );

	// @TODO stereo is not working at the moment
	// we need to render the window twice and we are at the moment using
	// the windows update system.
	// So when window is updated it should render twice
	// (for example using two viewports) once for each backbuffer
	if( _stereo )
	{
		//draw into back left buffer
		glDrawBuffer(GL_BACK_LEFT);
		Ogre::Vector3 eye(-_ipd/2, 0, 0);
		_setOgreFrustum(_camera);
		_setOgreView( _camera, eye );
		_viewport->update();

		//draw into back right buffer
		glDrawBuffer(GL_BACK_RIGHT);
		eye = Ogre::Vector3(_ipd/2, 0, 0);
		_setOgreFrustum(_camera);
		_setOgreView( _camera, eye );
		_viewport->update();
	}
	else
	{
		_setOgreFrustum(_camera);
		_setOgreView( _camera );
		_viewport->update();
	}
}

void
vl::Channel::_setOgreFrustum(Ogre::Camera *camera)
{
	assert( camera );
	assert( !_wall.empty() );

	Ogre::Real c_near = camera->getNearClipDistance();
	Ogre::Real c_far = camera->getFarClipDistance();

	Ogre::Matrix4 projMat = calculate_projection_matrix(c_near, c_far, _wall);
	camera->setCustomProjectionMatrix( true, projMat );
}

void
vl::Channel::_setOgreView( Ogre::Camera *camera, Ogre::Vector3 eye )
{
	assert( camera );

	// Get modified camera orientation

	/* Not in use, seems mostly silly and easier to deal with in user code
	Ogre::Quaternion cam_orient;

	// Y-axis should be fine for VR systems, X and Z are not.
	// NOTE if these rotations are disabled when moving the camera node
	// the moving direction is different than the direction the camera is facing
	// FIXME
	uint32_t cam_rot_flags(-1);// = getSettings().getCameraRotationAllowed();
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
	*/

	Ogre::Matrix4 viewMatrix = calculate_view_matrix(camera->getRealPosition(), 
		camera->getRealOrientation(), _wall, _head_matrix, eye);
	camera->setCustomViewMatrix(true, viewMatrix);
}
