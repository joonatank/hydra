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
#include <OGRE/OgreRectangle2D.h>

#include "stereo_camera.hpp"

namespace vl
{

/// @todo should these be flags instead? or is it too hard to implement
enum RENDER_MODE
{
	// Default mode render directly to screen
	RM_WINDOW,
	// Render to FBO first then Blit the FBO directly to screen
	RM_FBO,		// Working fine for both mono and stereo buffers
	// Use MRT to render both stereo images to FBO in a single pass
	RM_MRT_STEREO,	// Not implemented
	// Render first G-Buffer and then lights
	RM_DEFERRED,	// Work in progress
	// Combined mode
	RM_DEFERRED_MRT_STEREO,	// Not implemented
};

class Channel
{
public:
	Channel(vl::config::Channel config, Ogre::Viewport *view, RENDER_MODE rm, uint32_t fsaa);

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

	void _set_fbo_camera(vl::CameraPtr cam);

	void _initialise_fbo(vl::CameraPtr camera);

	void _set_mrt_camera(vl::CameraPtr cam);

	void _initialise_mrt(vl::CameraPtr camera);

	Ogre::RenderTexture *_create_fbo(vl::CameraPtr camera, std::string const &name, Ogre::PixelFormat pf);

	Ogre::SceneNode *_create_screen_quad(std::string const &name, std::string const &material_name);

	void _deferred_geometry_pass(void);

	void _deferred_light_pass(void);

	uint32_t _get_window_mask(void)
	{
		// minimum 1<<2 because 1<<0 is the scene mask
		// and 1<<1 is the light mask
		return 1 << (viewport->getZOrder()+2); 
	}

	uint32_t _get_scene_mask(void)
	{ return 1; }

	uint32_t _get_light_mask(void)
	{ return 1<<1; }

	// convenience function for fbo rendering
	uint32_t _get_fbo_mask(void)
	{ return _get_scene_mask() | _get_light_mask(); }

	/// Data
private:
	std::string _name;

	RENDER_MODE _render_mode;
	uint32_t _fsaa;

	STEREO_EYE _stereo_eye_cfg;
	vl::Player *_player;

	// Special Rendering surface
	// MRT is only available for Deferred shading
	// FBO is only available for FBO rendering
	Ogre::MultiRenderTarget *_mrt;
	Ogre::RenderTexture *_fbo;

	std::vector<Ogre::MaterialPtr> _fbo_materials;
	std::vector<Ogre::TexturePtr> _fbo_textures;

	Ogre::Camera *_rtt_camera;
};	// class Channel

}	// namespace vl

#endif	// HYDRA_CHANNEL_HPP
