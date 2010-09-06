
#include "channel.hpp"

#include "window.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent ) 
	: eq::Channel(parent), _ogre_window(0), _viewport(0), _camera(0),
	  _head_pos( Ogre::Vector3::ZERO ), _head_orient( Ogre::Quaternion::IDENTITY ), _tracker()
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
	_ogre_window = ((eqOgre::Window *)getWindow())->getRenderWindow();
	EQASSERT( _ogre_window );

	std::cerr << "Get camera from RenderWindow" << std::endl;
	_camera = ((eqOgre::Window *)getWindow())->getCamera();
	EQASSERT( _camera );

	std::cerr << "Creating viewport" << std::endl;
	_viewport = _ogre_window->addViewport( _camera );
	_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );

	// TODO this needs to be configurable
	setNearFar( 0.1, 100.0 );

	_tracker = ((eqOgre::Window *)getWindow())->getTracker();
	EQASSERT( _tracker );

	return true;
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
eqOgre::Channel::frameDraw( const uint32_t /*frameID */)
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

void 
eqOgre::Channel::setOgreFrustum( void )
{
	eq::Frustumf frust = getFrustum();
	_camera->setCustomProjectionMatrix( true, vl::math::convert( frust.compute_matrix() ) );
	// TODO add support for cameras with parent
	// Ogre::Camera::getPosition returns position relative to the parent
	Ogre::Matrix4 viewMat = Ogre::Math::makeViewMatrix( _camera->getPosition() + _head_pos, _camera->getOrientation() ); //Ogre::Quaternion::IDENTITY );
	_camera->setCustomViewMatrix( true, viewMat );
}

void
eqOgre::Channel::setHeadMatrix( void )
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