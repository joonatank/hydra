
#include "ogre_viewport.hpp"

// library includes
#include "base/exceptions.hpp"

vl::ogre::Viewport::Viewport(Ogre::Viewport* view)
	: _ogre_viewport( view )
{
	if( !_ogre_viewport )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}
}

vl::ogre::Viewport::~Viewport(void )
{
}

Ogre::Viewport* vl::ogre::Viewport::getNative(void )
{
	// We should never end up here without ogre Viewport
	assert( _ogre_viewport );

	return _ogre_viewport;
}

void vl::ogre::Viewport::clear(void )
{
	// We should never end up here without ogre Viewport
	assert( _ogre_viewport );

	_ogre_viewport->clear();
}

void vl::ogre::Viewport::update(void )
{
	// We should never end up here without ogre Viewport
	assert( _ogre_viewport );
	
	_ogre_viewport->update();
}

vl::graph::CameraRefPtr vl::ogre::Viewport::getCamera(void )
{ return vl::graph::CameraRefPtr(); }

vl::graph::RenderWindowRefPtr vl::ogre::Viewport::getTarget(void )
{ return vl::graph::RenderWindowRefPtr(); }

void vl::ogre::Viewport::setCamera(vl::graph::CameraRefPtr cam)
{

}

int vl::ogre::Viewport::getZOrder(void )
{
	// We should never end up here without ogre Viewport
	assert( _ogre_viewport );

	return _ogre_viewport->getZOrder();
}

void vl::ogre::Viewport::setBackgroundColour(const vl::colour& colour)
{
	// We should never end up here without ogre Viewport
	assert( _ogre_viewport );

	Ogre::ColourValue c( colour.r(), colour.g(), colour.b(),
			colour.a() );
	_ogre_viewport->setBackgroundColour(c);
}
