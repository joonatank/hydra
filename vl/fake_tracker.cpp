#include "fake_tracker.hpp"

vl::FakeTracker::FakeTracker( void )
{}

vl::FakeTracker::~FakeTracker( void )
{}

void 
vl::FakeTracker::init( void )
{
}

size_t 
vl::FakeTracker::getNSensors( void ) const
{ return _data.size(); }

Ogre::Vector3 const &
vl::FakeTracker::getPosition( size_t sensor ) const
{ return _data.at(sensor).position; }

Ogre::Quaternion const &
vl::FakeTracker::getOrientation( size_t sensor ) const
{ return _data.at(sensor).quaternion; }

void 
vl::FakeTracker::mainloop( void )
{}

void
vl::FakeTracker::setPosition( size_t sensor, Ogre::Vector3 const &pos )
{
	if( _data.size() <= sensor )
	{ _data.resize( sensor + 1 ); }

	_data.at(sensor).position = pos;
}

void 
vl::FakeTracker::setOrientation( size_t sensor, Ogre::Quaternion const &quat )
{
	if( _data.size() <= sensor )
	{ _data.resize( sensor + 1 ); }

	_data.at(sensor).quaternion = quat;
}
