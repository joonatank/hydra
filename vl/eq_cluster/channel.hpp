/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *	2011-02 Update
 *	Removed Equalizer
 */

#ifndef VL_CHANNEL_HPP
#define VL_CHANNEL_HPP

#include <OGRE/OgreCamera.h>

#include "base/envsettings.hpp"

namespace vl
{

/**
 * The rendering entity, updating a part of a Window.
 */
class Channel
{
public:
	/// @brief Create a channel
	/// @TODO we should have a default wall to use if one is not provided
    Channel( vl::EnvSettings::Channel const &chanConf, 
			 vl::EnvSettings::Wall const &wall, double ipd );

    virtual ~Channel (void);

	void setCamera( Ogre::Camera *cam );

	void setHeadMatrix( Ogre::Matrix4 const &m )
	{ _head_matrix = m; }

	std::string getName( void ) const
	{ return _channel_conf.name; }

	virtual void draw( void );

protected:

	// Some task methods
	// The eye is not a reference because it's going to get modified
	void _setOgreFrustum( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );
	void _setOgreView( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );

	/// Draw settings
	vl::EnvSettings::Channel _channel_conf;
	vl::EnvSettings::Wall _wall;
	bool _stereo;
	double _ipd;
	
	/// Ogre variables
	Ogre::Camera *_camera;

	Ogre::Matrix4 _head_matrix;

};	// class Channel

}	// namespace vl

#endif // VL_CHANNEL_HPP

