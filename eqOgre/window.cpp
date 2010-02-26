#include "window.hpp"

#include <fstream>
#include <sstream>

#include <eq/client/osWindow.h>

//#include <OgreMovableObject.h>

#include "pipe.hpp"
#include "config.hpp"
#include "graph/root.hpp"
#include "channel.hpp"

#include "math/conversion.hpp"

unsigned int eqOgre::Window::n_windows = 0;

bool
eqOgre::Window::configInit( const uint32_t initID )
{
	if( !eq::Window::configInit( initID ))
	{ return false; }

//	createWindow();
//	createViewports();

	return true;
}

bool
eqOgre::Window::configInitGL( const uint32_t initID )
{
	if( !eq::Window::configInitGL( initID ))
	{ return false; }

    return true;
}

bool
eqOgre::Window::configExitGL()
{
	return eq::Window::configExitGL();
}

void
eqOgre::Window::frameStart( const uint32_t frameID,
		const uint32_t frameNumber )
{
	if( _state == 0 )
	{
		/*
		eq::OSWindow *win = getOSWindow();
		// For linux (or *nix in general)
		if( eq::GLXWindow *glx_w = dynamic_cast<eq::GLXWindow *>( win ) )
		{
			EQINFO << "Current GLXContext = " << glx_w->getGLXContext()
				<< std::endl;
		}
		*/
		//createWindow();
		//createViewports();

		_state++;
	}

//	if( _state > 0 )
//	{ loadMeshes(); }

	// TODO remove this after we have finished updating channels
	// to do all rendering using viewports
	if( _render_window )
	{ _render_window->update( ); }

	eq::Window::frameStart( frameID, frameNumber );
}

void
eqOgre::Window::swapBuffers( void )
{
	// TODO check if necessary after implemented channel based rendering
	if( _render_window )
	{ _render_window->swapBuffers(); }

	eq::Window::swapBuffers();
}

void
eqOgre::Window::createViewports( void )
{
	if( !_render_window )
	{
		EQERROR << "No Ogre Rendering window defined" << std::endl;
		return;
	}

	// TODO creation and attachment of objects should be moved to Node
	/*
	 * FIXME camera creation removed for now, we need to use
	 * bit more sophisticated method for it.
	 * So cameras and windows should be created in Node (like all other 
	 * objects).
	EQINFO << "Setting hard-coded camera to all channels" << std::endl;
	vl::graph::Root *root = ((eqOgre::Pipe *)getPipe() )->getRoot();
	EQASSERT( root );
	vl::graph::SceneManager *sm = root->getSceneManager();
	EQASSERT( sm );

	Ogre::String name = "Cam-" + Ogre::StringConverter::toString( n_windows );
	_camera = sm->createCamera( name );

	Ogre::SceneNode *feet = 0;
	if( sm->hasSceneNode( "Feet" ) )
	{ feet = sm->getSceneNode( "Feet" ); }
	else
	{ feet = sm->createSceneNode( "Feet" ); }

	feet->lookAt( Ogre::Vector3(0, 0, 50), Ogre::Node::TS_WORLD );
	feet->attachObject( _camera );
	*/
//	EQINFO << "ogre projection = " << _camera->getProjectionMatrix() << std::endl;
//	EQINFO << "ogre near clip = " << _camera->getNearClipDistance() << std::endl;
//	EQINFO << "ogre far clip = " << _camera->getFarClipDistance() << std::endl;
//	chan->setCamera( cam );

	/*
	EQINFO << "Creating viewport" << std::endl;
	Ogre::Viewport *ogre_vp =
		_ogre_window->addViewport( _camera );
	ogre_vp->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0) );
	*/
}

void
eqOgre::Window::createWindow( void )
{
	/*
	// We shouldn't create multiple windows
	if( _ogre_window )
	{
		EQERROR << "createWindow : Window already created." << std::endl;
		return;
	}

	eqOgre::Pipe *pipe= static_cast<eqOgre::Pipe *>( getPipe() );
	EQASSERT( pipe );
	EQASSERT( pipe->getRoot() );

	// Create the Ogre Rendering Window
	Ogre::NameValuePairList params;

	eq::OSWindow *win = getOSWindow();
	// For linux (or *nix in general)
	if( eq::GLXWindow *glx_w = dynamic_cast<eq::GLXWindow *>( win ) )
	{
		EQINFO << "Current GLXContext = " << glx_w->getGLXContext()
			<< std::endl;
		params["currentGLContext"] = Ogre::String("True");
		params["parentWindowHandle"]
			= Ogre::StringConverter::toString( glx_w->getXDrawable() );
	}
	else
	{
		EQERROR << "Sorry only linux support for now" << std::endl;
		EQASSERT( false );
	}
	*/
	// For windows
	// TODO implement
	/*
	else if( WGLWindowIF *wgl = dynamic_cast<WGLWindowIF *>( win ) )
	{
		params["externalWindowHandle"] = StringConverter::toString(winHandle);
		params["externalGLContext"] = StringConverter::toString(winGlContext);
	}
	// For AGL (Mac OS X), no support atm
	else if( AGLWindowIF *agl = dynamic_cast<AGLWindowIF *>( win ) )
	{
		EQERROR << "AGL support is not yet implemented, sorry."
			<< std::endl;
		// TODO replace this with real quit
		EQASSERT( false );
	}
	*/

	/*
	eq::PixelViewport const &pvp = getPixelViewport();
	EQASSERT( pvp.hasArea() );
	EQINFO << "Creating Ogre::Window" << std::endl;
	EQINFO << "Window rect = " <<  pvp.x << " " << pvp.y << " "
		<< pvp.w << " "<< pvp.h << std::endl;

	Ogre::String win_name
		= "Window-" + Ogre::StringConverter::toString( n_windows );
	n_windows++;

	_ogre_window = pipe->getRoot()
		->createWindow( win_name, pvp.w, pvp.h, false, &params);

	// We need to initialise resources here, because this function needs
	// Windows to be already created.
	// Also we don't do it more than once for every pipe thread.
	// Might be better to do it once in eq::Node (separate thread),
	// but we need to find suitable place to hook it up.
	// So that eq::Window::configGLInit has been called.
	if( _ogre_window && n_windows == 1 )
	{
		EQINFO << "Initialisin resources" << std::endl;
		Ogre::ResourceGroupManager::getSingleton()
			.initialiseAllResourceGroups();
	}
	*/
}


void
eqOgre::Window::setFrustum( vmml::mat4d const & )
{
	/*
	if( !_camera )
	{ return; }

    Ogre::Matrix4 projMat vl::math::convert(m);

    _camera->setCustomProjectionMatrix(true, projMat);
	*/
}

void
eqOgre::Window::loadMeshes( void )
{
	/*
	if( !_ogre_window )
	{ return; }

	eqOgre::Pipe *pipe= static_cast<eqOgre::Pipe *>( getPipe() );
	Ogre::MovableObjectFactory *fac =
		pipe->getRoot()->getMovableObjectFactory( "Entity" );
	EQASSERT( dynamic_cast<eqOgre::EntityFactory *>( fac ) );
	static_cast<eqOgre::EntityFactory *>( fac )->finalize();
	*/
}

