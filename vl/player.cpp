/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 */
#include "player.hpp"

#include "math/conversion.hpp"

// TODO we should pass the active camera to this, so it's not arbitary
vl::Player::Player( void )
	: _screenshot_version(0)
	, _ipd(0)
{}

vl::Player::~Player( void )
{}

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
vl::Player::setHeadTransform(vl::Transform const &m)
{
	setDirty( DIRTY_HEAD );
	_head_transform = m;
}

void
vl::Player::takeScreenshot( void )
{
	setDirty( DIRTY_SCREENSHOT );
	_screenshot_version++;
}

void
vl::Player::setIPD(double ipd)
{
	if( _ipd != ipd )
	{
		setDirty(DIRTY_IPD);
		_ipd = ipd;
	}
}

/// ------------------------------- Protected ----------------------------------
void
vl::Player::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_HEAD )
	{ msg << _head_transform; }

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{ msg << _active_camera; }

	if( dirtyBits & DIRTY_SCREENSHOT )
	{ msg << _screenshot_version; }

	if( dirtyBits & DIRTY_IPD )
	{ msg << _ipd; }
}

void
vl::Player::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_HEAD )
	{ msg >> _head_transform; }

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{ msg >> _active_camera; }

	if( dirtyBits & DIRTY_SCREENSHOT )
	{ msg >> _screenshot_version; }

	if( dirtyBits & DIRTY_IPD )
	{ msg >> _ipd; }
}
