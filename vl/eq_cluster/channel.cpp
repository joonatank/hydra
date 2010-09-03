
#include "channel.hpp"

#include "window.hpp"

#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent ) 
	: eq::Channel( parent )
{}

eqOgre::Channel::~Channel( void )
{

}

/*
void
eqOgre::Channel::setCamera( eqOgre::Camera *cam )
{
	EQINFO << "Camera set to Channel" << std::endl;

	_camera = cam;

	if( _camera )
	{
		_camera->setNearClipDistance(5);

		if( _ogre_viewport )
		{
			_ogre_viewport->setCamera( cam );
			_camera->setAspectRatio(
					_ogre_viewport->getWidth()/_ogre_viewport->getHeight() );
		}
	}
}

void
eqOgre::Channel::setViewport( Ogre::Viewport *viewport )
{
	EQINFO << "Viewport set to Channel" << std::endl;
	_ogre_viewport = viewport; 
	if( _ogre_viewport && _camera )
	{
		_ogre_viewport->setCamera( _camera );
		_camera->setAspectRatio(
				_ogre_viewport->getWidth()/_ogre_viewport->getHeight() );
	}
}
*/

bool
eqOgre::Channel::configInit( const uint32_t initID )
{
	if ( !eq::Channel::configInit( initID ) )
	{ return false; }

	// Using the Ogre settings for now
    setNearFar( 100.0f, 100.0e3f );

    return true;
}

void
eqOgre::Channel::frameClear( const uint32_t /*frameID */)
{
//	TODO channel should do all rendering tasks
//	it should use Ogre::Viewport to do so.
//	if( _camera && _ogre_viewport )
//	{ _ogre_viewport->clear(); }
}

/** Override frameDraw to call Viewport::update
 *
 *  Original does applyBuffer, applyViewport, applyFrustum, applyHeadTransform
 */
void
eqOgre::Channel::frameDraw( const uint32_t /*frameID */)
{
	// applyBuffer

	// applyViewport

	// applyFrustum
	// Lets set it here straight to the camera
//	applyFrustum();
	
	// applyHeadTransform
	// Equalizer method which handles the tracker head
	applyHeadTransform();

	// Draw the viewport

}

/* Override the applyFrustum to set Camera Frustum
void
eqOgre::Channel::applyFrustum() const
{
	// Set the Camera frusrum
//	eqOgre::Window* pWin = (eqOgre::Window *)getWindow();

	eq::Frustumf frust = getFrustum();
	// Apply the frustum to Ogre::Camera
//	pWin->setFrustum( frust.compute_matrix() );
}
*/
