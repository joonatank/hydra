
#include "channel.hpp"

//#include "initData.h"
#include "config.hpp"
#include "pipe.hpp"
//#include "root.hpp"
//#include "view.hpp"
#include "window.hpp"
//#include "vertexBufferState.hpp"

/*
// light parameters
static GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f};
static GLfloat lightAmbient[]  = {0.1f, 0.1f, 0.1f, 1.0f};
static GLfloat lightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
static GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};

// material properties
static GLfloat materialAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
static GLfloat materialDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
static GLfloat materialSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
static GLint  materialShininess   = 64;
*/

#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

#include "window.hpp"

#include "channel.hpp"

eqOgre::Channel::Channel( eq::Window *parent ) 
	: eq::Channel( parent )
//	  _root(0),
//	  _ogre_viewport(0),
//	  _camera(0)
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

void
eqOgre::Channel::frameDraw( const uint32_t /*frameID */)
{
	// Set the Ogre Camera frusrum to same as equalizer
	eqOgre::Window* pWin = (eqOgre::Window *)getWindow();

	eq::Frustumf frust = getFrustum();
	pWin->setFrustum( frust.compute_matrix() );

	/*
	static bool inited = false;

	if( !inited )
	{
		EQINFO << "eq frust matrix = " << frust.compute_matrix() << std::endl;
		EQINFO << "eq near clip = " << frust.near_plane() << std::endl;
		EQINFO << "eq far clip = " << frust.far_plane() << std::endl;
		inited = true;
	}
	*/


	// TODO this should update Ogre::Viewport from equalizer Viewport
	
	// TODO this should call Ogre::Viewport::render
	// All rendering should be done from here.
}

void
eqOgre::Channel::frameReadback( const uint32_t frameID )
{
    eq::Channel::frameReadback( frameID );
}

void
eqOgre::Channel::applyFrustum() const
{
	// Empty function because we are setting the Frustum from frameDraw
//	eq::Channel::applyFrustum();
}

void
eqOgre::Channel::frameViewFinish( const uint32_t frameID )
{
	eq::Channel::frameViewFinish( frameID );
}

