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

	void update(void);

	void draw(void);

	StereoCamera camera;
	Ogre::Viewport *viewport;

	STEREO_EYE getStereoEyeCfg(void) const
	{ return _stereo_eye_cfg; }

	void setStereoEyeCfg(STEREO_EYE eye_cfg)
	{ _stereo_eye_cfg = eye_cfg; }
	
	/// Necessary to provide getter/setter because player does not exists
	/// when the Channel is created.
	Player *getPlayer(void) const
	{ return _player; }

	void setPlayer(Player *player)
	{ _player = player; }

	/// Per frame statistics
	vl::scalar getLastFPS(void) const;

	size_t getTriangleCount(void) const;

	size_t getBatchCount(void) const;

	/// Methods
private :

	void _render_to_fbo(void);

	void _initialise_fbo(vl::CameraPtr camera);
	
	/// Data
private:
	std::string _name;

	bool _use_fbo;

	STEREO_EYE _stereo_eye_cfg;
	vl::Player *_player;

	Ogre::RenderTexture *_fbo;
	Ogre::MaterialPtr _fbo_material;
	Ogre::TexturePtr _fbo_texture;

};	// class Channel

}	// namespace vl

#endif	// HYDRA_CHANNEL_HPP
