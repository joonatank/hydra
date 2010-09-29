
#include "channel.hpp"

#include "window.hpp"
#include "config.hpp"

#include "math/conversion.hpp"
#include "base/exceptions.hpp"

eqOgre::Channel::Channel( eq::Window *parent ) 
	: eq::Channel(parent), _ogre_window(0), _viewport(0), _camera(0), _ogre_node(0),
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
	eqOgre::Window *window = ((eqOgre::Window *)getWindow());
	_ogre_window = window->getRenderWindow();
	EQASSERT( _ogre_window );

	std::cerr << "Get camera from RenderWindow" << std::endl;
	_camera = ((eqOgre::Window *)getWindow())->getCamera();
	EQASSERT( _camera );
	_camera_initial_position = _camera->getPosition();
	_camera_initial_orientation = _camera->getOrientation();

	// Get the ogre node
	EQASSERT( window->getSceneManager() );
	
	if( window->getSceneManager()->hasSceneNode("ogre") )
	{
		_ogre_node = window->getSceneManager()->getSceneNode( "ogre" );
		_ogre_initial_position = _ogre_node->getPosition(); 
		_ogre_initial_orientation = _ogre_node->getOrientation();
	}
	else
	{
		_ogre_initial_position = Ogre::Vector3::ZERO;
		_ogre_initial_orientation = Ogre::Quaternion::IDENTITY;
	}

	std::cerr << "Creating viewport" << std::endl;
	_viewport = _ogre_window->addViewport( _camera );
	_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );

	// TODO this needs to be configurable
	setNearFar( 0.1, 100.0 );

	// Get framedata
	eqOgre::Config *config = dynamic_cast< eqOgre::Config * >( getConfig() );
	if( !config )
	{
		EQERROR << "config is not type eqOgre::Config" << std::endl;
		return false;
	}
	
	EQASSERT( config->getInitData().getFrameDataID() != EQ_ID_INVALID );
	config->mapObject( &_frame_data, config->getInitData().getFrameDataID() );
	
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
eqOgre::Channel::frameDraw( const uint32_t frameID )
{
	// Distribution
	_frame_data.sync( frameID );
	updateDistribData();

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

void 
eqOgre::Channel::updateDistribData( void )
{
	_camera->setPosition( _frame_data.getCameraPosition()+_camera_initial_position );
	_camera->setOrientation( _frame_data.getCameraRotation()*_camera_initial_orientation );

	if( _ogre_node )
	{
		_ogre_node->setPosition( _frame_data.getOgrePosition()+_ogre_initial_position );
		_ogre_node->setOrientation( _frame_data.getOgreRotation()*_ogre_initial_orientation );
	}
}
