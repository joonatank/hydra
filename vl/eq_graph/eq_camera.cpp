#include "eq_camera.hpp"

// 
vl::cl::Camera::Camera( std::string name, vl::NamedValuePairList const &params )
		: _name( name ), _manager()
{}

void 
vl::cl::Camera::setManager( vl::graph::SceneManagerRefPtr man )
{
	std::cerr << "vl::cl::Cmaera::setManager." << std::endl;
	if( !man )
	{
		throw vl::null_pointer( "vl::cl::Camera::setManager" );
	}

	_manager = man;
}

// ---------- CameraFactory ----------
vl::graph::MovableObjectRefPtr
vl::cl::CameraFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	std::cerr << "vl::cl::CameraFactory::create" << std::endl;
	vl::graph::MovableObjectRefPtr obj( new vl::cl::Camera( name, params) );
	return obj;
}
			
const std::string vl::cl::CameraFactory::TYPENAME = "Camera";
