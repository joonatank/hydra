
#include "frame_data.hpp"

// ------ Protected --------
void 
eqOgre::FrameData::getInstanceData( eq::net::DataOStream& os )
{
	// Serialize camera position
	os << _camera_pos;

	// Serialize camera orientation
	os << _camera_rotation;

	// Serialize ogre position
	os << _ogre_pos;

	// Serialize ogre orientation
	os << _ogre_rotation;
}

void 
eqOgre::FrameData::applyInstanceData( eq::net::DataIStream& is )
{
	// Serialize camera position
	is >> _camera_pos;

	// Serialize camera orientation
	is >> _camera_rotation;

	// Serialize ogre position
	is >> _ogre_pos;

	// Serialize ogre orientation
	is >> _ogre_rotation;
}