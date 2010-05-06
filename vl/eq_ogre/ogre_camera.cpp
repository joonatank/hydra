#include "ogre_camera.hpp"

#include "ogre_scene_manager.hpp"

vl::ogre::Camera::Camera( std::string name, vl::NamedValuePairList const &params )
	: vl::cl::Camera( name, params ), _ogre_camera(0)
{}

void
vl::ogre::Camera::setManager( vl::graph::SceneManagerRefPtr man )
{
	const char *here = "vl::ogre::Camera::setManager."; 
	std::cerr << here << std::endl;
	vl::cl::Camera::setManager( man );

	boost::shared_ptr<SceneManager> og_man = 
		boost::dynamic_pointer_cast<vl::ogre::SceneManager>( man );
	if( !og_man )
	{ throw vl::bad_cast( here ); }
	if( !(og_man->getNative()) )
	{
		throw vl::exception( "Manager has no native.", here );
	}

	std::cerr << "vl::ogre::Camera::setManager : creating ogre camera." << std::endl;
	
	_ogre_camera = og_man->getNative()->createCamera( _name );
	if( !_ogre_camera)
	{ throw vl::null_pointer( here ); }
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

Ogre::MovableObject *
vl::ogre::Camera::getNative( void )
{ return _ogre_camera; }

// ---------- CameraFactory	----------
vl::graph::MovableObjectRefPtr
vl::ogre::CameraFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	std::cerr << "vl::ogre::CameraFactory::create" << std::endl;
	vl::graph::MovableObjectRefPtr obj( new vl::ogre::Camera( name, params) );
	return obj;
}

const std::string vl::ogre::CameraFactory::TYPENAME = "Camera";