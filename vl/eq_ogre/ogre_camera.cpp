#include "ogre_camera.hpp"

#include "ogre_scene_manager.hpp"

void
vl::ogre::Camera::setManager( vl::graph::SceneManagerRefPtr man )
{
	vl::cl::Camera::setManager( man );

	boost::shared_ptr<SceneManager> og_man = 
		boost::dynamic_pointer_cast<vl::ogre::SceneManager>( man );
	if( !og_man )
	{ throw vl::null_pointer( "vl::ogre::Camera::Camera" ); }

	_ogre_camera = og_man->getNative()->createCamera( _name );

	if( !_ogre_camera)
	{ throw vl::null_pointer( "vl::ogre::Camera::Camera" ); }
}

void
vl::ogre::Camera::setProjectionMatrix( vl::matrix const &m )
{
	if( _ogre_camera )
	{
		_ogre_camera->setCustomProjectionMatrix( true,
				vl::math::convert(m) );
	}
}

void
vl::ogre::Camera::setFarClipDistance( vl::scalar const &dist )
{
	if( _ogre_camera )
	{ _ogre_camera->setFarClipDistance( dist ); }
}

void
vl::ogre::Camera::setNearClipDistance( vl::scalar const &dist )
{
	if( _ogre_camera )
	{ _ogre_camera->setNearClipDistance( dist ); }
}

void
vl::ogre::Camera::setPosition( vl::vector const &pos )
{
	if( _ogre_camera )
	{ _ogre_camera->setPosition( vl::math::convert( pos ) ); }
}

// ---------- CameraFactory	----------
vl::graph::MovableObjectRefPtr
vl::ogre::CameraFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	vl::graph::MovableObjectRefPtr obj( new vl::ogre::Camera( name, params) );
	return obj;
}

const std::string vl::ogre::CameraFactory::TYPENAME = "Camera";
