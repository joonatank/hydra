
#include "frame_data.hpp"

/// Public
uint32_t eqOgre::FrameData::commitAll(void )
{
	if( _ogre.isDirty() )
	{ _ogre.commit(); }

	if( isDirty() )
	{ return commit(); }
	else
	{ return getVersion(); }
}

void
eqOgre::FrameData::syncAll(void )
{
	_ogre.sync();
	sync();
}

void
eqOgre::FrameData::registerData(eq::Config* config)
{
	// Lets make sure we don't register the objects more than once
	if( EQ_ID_INVALID != _ogre_id && EQ_ID_INVALID != _ogre.getID() )
	{
		std::cerr << "Ogre already registered" << std::endl;
		EQASSERT( false );
	}
	config->registerObject( &_ogre );
	// The object has to be correctly registered
	if( EQ_ID_INVALID == _ogre.getID() )
	{
		std::cerr << "Ogre was not registered" << std::endl;
		EQASSERT( false );
	}
	_ogre_id = _ogre.getID();

	if( EQ_ID_INVALID != getID() )
	{
		std::cerr << "FrameData already registered" << std::endl;
		EQASSERT( false );
	}
	config->registerObject( this );
	if( EQ_ID_INVALID == getID() )
	{
		std::cerr << "FrameData was not registered" << std::endl;
		EQASSERT( false );
	}
}

void
eqOgre::FrameData::deregisterData(eq::Config* config)
{
	if( EQ_ID_INVALID != _ogre.getID() )
	{ config->deregisterObject( &_ogre ); }

	_ogre_id = EQ_ID_INVALID;

	if( EQ_ID_INVALID != getID() )
	{ config->deregisterObject( this ); }
}

void
eqOgre::FrameData::mapData(eq::Config* config, uint32_t id)
{
	if( EQ_ID_INVALID == id )
	{
		std::cerr << "Trying to map to invalid ID" << std::endl;
		EQASSERT( false );
	}
	config->mapObject(this, id);

	if( EQ_ID_INVALID == _ogre_id )
	{
		std::cerr << "Trying to map Ogre to invalid ID" << std::endl;
		EQASSERT( false );
	}
	config->mapObject(&_ogre, _ogre_id);
}

void
eqOgre::FrameData::unmapData(eq::Config* config)
{
	config->unmapObject( &_ogre );
	config->unmapObject( this );
}


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
