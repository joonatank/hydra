
#include "frame_data.hpp"

// ------ Protected --------
void 
eqOgre::FrameData::serialize( eq::net::DataOStream &os, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::serialize( os, dirtyBits );

	if( dirtyBits & DIRTY_CAMERA ) 
	{
		// Serialize camera position
		operator<<( _camera_pos, os );
		// Serialize camera orientation
		operator<<( _camera_rotation, os );
	}

	if( dirtyBits & DIRTY_OGRE )
	{
		os << _ogre_id;
		// Serialize ogre position
//		operator<<( _ogre_pos, os );
		// Serialize ogre orientation
//		operator<<( _ogre_rotation, os );
	}

	if( dirtyBits & DIRTY_RELOAD )
	{
		os << _scene_version;
	}
}

void 
eqOgre::FrameData::deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::deserialize( is, dirtyBits );

	if( dirtyBits & DIRTY_CAMERA )
	{
		// Serialize camera position
		operator>>( _camera_pos, is );

		// Serialize camera orientation
		operator>>( _camera_rotation, is );
	}

	if( dirtyBits & DIRTY_OGRE )
	{
		is >> _ogre_id;
	}

	if( dirtyBits & DIRTY_RELOAD )
	{
		is >> _scene_version;
	}
}
