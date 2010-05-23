
// interface
#include "eq_camera.hpp"

// library includes
#include "base/exceptions.hpp"

// ---------- Camera -----------
vl::cl::Camera::Camera( std::string name, vl::NamedValuePairList const &params )
		: _manager(), _name( name )
{}

std::string const&
vl::cl::Camera::getTypename(void ) const
{ return CameraFactory::TYPENAME; }

void 
vl::cl::Camera::setManager( vl::graph::SceneManagerRefPtr man )
{
	std::cerr << "vl::cl::Cmaera::setManager." << std::endl;
	if( !man )
	{
		throw vl::null_pointer();
	}

	_manager = man;
}

// ---------- CameraFactory ----------
vl::graph::MovableObjectRefPtr
vl::cl::CameraFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	vl::graph::MovableObjectRefPtr obj( new vl::cl::Camera( name, params) );
	return obj;
}
			
const std::string vl::cl::CameraFactory::TYPENAME = "Camera";
