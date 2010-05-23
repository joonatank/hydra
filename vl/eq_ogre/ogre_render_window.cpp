
#include "ogre_render_window.hpp"

// library includes
#include "base/exceptions.hpp"
#include "ogre_viewport.hpp"
#include "ogre_camera.hpp"

vl::ogre::RenderWindow::RenderWindow(Ogre::RenderWindow* win)
	: _ogre_window( win )
{
	if( !_ogre_window )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}
}

vl::ogre::RenderWindow::~RenderWindow(void )
{

}

Ogre::RenderWindow* vl::ogre::RenderWindow::getNative()
{ return _ogre_window; }

void vl::ogre::RenderWindow::swapBuffers(void )
{
	// We should have ogre Window here
	assert( _ogre_window );

	_ogre_window->swapBuffers( false );
}

void vl::ogre::RenderWindow::update(void )
{
	// We should have ogre Window here
	assert( _ogre_window );

	_ogre_window->update( false );
}

vl::graph::ViewportRefPtr
vl::ogre::RenderWindow::addViewport(vl::graph::CameraRefPtr cam)
{
	// We should have ogre Window here
	assert( _ogre_window );
	
	boost::shared_ptr<Camera> c
		= boost::dynamic_pointer_cast<Camera>( cam );
	if( !c )
	{
		BOOST_THROW_EXCEPTION( vl::cast_error() );
	}

	Ogre::Viewport *ogre_view = _ogre_window->addViewport(
				(Ogre::Camera *)c->getNative() );
	vl::graph::ViewportRefPtr view( new Viewport( ogre_view ) );
	_viewports.push_back(view);

	return view;
}

uint16_t
vl::ogre::RenderWindow::getNumViewports(void ) const
{
	// We should have ogre Window here
	assert( _ogre_window );
	
	return _ogre_window->getNumViewports();
}

vl::graph::ViewportRefPtr
vl::ogre::RenderWindow::getViewport(uint16_t index)
{
	return vl::graph::ViewportRefPtr();
}

vl::graph::ViewportRefPtr
vl::ogre::RenderWindow::getViewportByZOrder(int ZOrder)
{
	return vl::graph::ViewportRefPtr();
}

bool
vl::ogre::RenderWindow::hasViewportWithZOrder(int ZOrder)
{
	return false;
}

void vl::ogre::RenderWindow::removeViewport(int ZOrder)
{
}
