#include "eq_camera.hpp"

// ---------- CameraFactory ----------
vl::graph::MovableObjectRefPtr
vl::cl::CameraFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	vl::graph::MovableObjectRefPtr obj( new vl::cl::Camera( name, params) );
	return obj;
}
			
const std::string vl::cl::CameraFactory::TYPENAME = "Camera";
