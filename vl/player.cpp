/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 */
#include "player.hpp"

#include "math/conversion.hpp"

vl::Player::Player( eq::Observer *observer )
	: _observer(observer)
{

}

vl::Player::~Player( void )
{

}

std::string const &
vl::Player::getActiveCamera( void ) const
{
	return _active_camera;
}

void
vl::Player::setActiveCamera(const std::string& name)
{
	_active_camera = name;
	std::cerr << "This should change the camera" << std::endl;
}

void
vl::Player::setHeadMatrix( Ogre::Matrix4 const &m )
{
	EQASSERT( _observer );
	// When head matrix is set equalizer automatically applies it to the
	// GL Modelview matrix as first transformation
	_observer->setHeadMatrix( vl::math::convert(m) );
}
