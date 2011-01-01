/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 */
#include "player.hpp"

#include "math/conversion.hpp"

// TODO we should pass the active camera to this, so it's not arbitary
vl::Player::Player( eq::Observer *observer )
	: _observer(observer)
{}

vl::Player::~Player( void )
{}

std::string const &
vl::Player::getActiveCamera( void ) const
{
	return _active_camera;
}

void
vl::Player::setActiveCamera( std::string const &name )
{
	if( _active_camera != name )
	{
		_active_camera = name;
		setDirty( DIRTY_ACTIVE_CAMERA );
	}
}

void
vl::Player::setHeadMatrix( Ogre::Matrix4 const &m )
{
	EQASSERT( _observer );
	// When head matrix is set equalizer automatically applies it to the
	// GL Modelview matrix as first transformation
	_observer->setHeadMatrix( vl::math::convert(m) );
}


/// ------------------------------- Protected ----------------------------------
void
vl::Player::serialize(co::DataOStream& os, const uint64_t dirtyBits)
{
	eq::fabric::Serializable::serialize(os, dirtyBits);

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{
		os << _active_camera;
	}
	if( dirtyBits & DIRTY_CAMERA_ENABLE_ROT )
	{
		// TODO add camera enable rotation here
	}
}

void
vl::Player::deserialize(co::DataIStream& is, const uint64_t dirtyBits)
{
	eq::fabric::Serializable::deserialize(is, dirtyBits);

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{
		is >> _active_camera;
	}
	if( dirtyBits & DIRTY_CAMERA_ENABLE_ROT )
	{
	}
}
