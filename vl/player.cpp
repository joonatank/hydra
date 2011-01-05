/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 */
#include "player.hpp"

#include "math/conversion.hpp"

// TODO we should pass the active camera to this, so it's not arbitary
vl::Player::Player( void )
	: _head_matrix(Ogre::Matrix4::IDENTITY), _screenshot_version(0)
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
	setDirty( DIRTY_HEAD );
	_head_matrix = m;
}

void
vl::Player::takeScreenshot( void )
{
	setDirty( DIRTY_SCREENSHOT );
	_screenshot_version++;
}


/// ------------------------------- Protected ----------------------------------
void
vl::Player::serialize(co::DataOStream& os, const uint64_t dirtyBits)
{
	eq::fabric::Serializable::serialize(os, dirtyBits);

	if( dirtyBits & DIRTY_HEAD )
	{ os << _head_matrix; }

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{ os << _active_camera; }

	if( dirtyBits & DIRTY_SCREENSHOT )
	{ os << _screenshot_version; }
}

void
vl::Player::deserialize(co::DataIStream& is, const uint64_t dirtyBits)
{
	eq::fabric::Serializable::deserialize(is, dirtyBits);

	if( dirtyBits & DIRTY_HEAD )
	{ is >> _head_matrix; }

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{ is >> _active_camera; }

	if( dirtyBits & DIRTY_SCREENSHOT )
	{ is >> _screenshot_version; }
}
