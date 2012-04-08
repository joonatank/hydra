/**
 *	Copyright (c) 2011-2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file channel.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#ifndef HYDRA_CHANNEL_HPP
#define HYDRA_CHANNEL_HPP


// Necessary for Window config and Wall
#include "base/envsettings.hpp"

#include <OGRE/OgreViewport.h>
#include <OGRE/OgreTexture.h>

#include "stereo_camera.hpp"

namespace vl
{

class Channel
{
public:
	Channel(vl::config::Channel config, Ogre::Viewport *view, bool use_fbo);

	void setCamera(vl::CameraPtr cam);

	std::string const &getName(void) const
	{ return _name; }

	void update(vl::Player const &player);

	void draw(double ipd, bool eye_left = true);

	StereoCamera camera;
	Ogre::Viewport *viewport;

	/// Methods
private :

	void _render_to_fbo(void);

	void _initialise_fbo(vl::CameraPtr camera);
	
	/// Data
private:
	std::string _name;

	bool _use_fbo;

	Ogre::RenderTexture *_fbo;
	Ogre::MaterialPtr _fbo_material;
	Ogre::TexturePtr _fbo_texture;

};	// class Channel

}	// namespace vl

#endif	// HYDRA_CHANNEL_HPP
