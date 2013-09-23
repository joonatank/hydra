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
#include "base/config.hpp"

#include <OGRE/OgreViewport.h>
#include <OGRE/OgreTexture.h>
#include <OGRE/OgreRectangle2D.h>

#include "stereo_camera.hpp"
// necessary for Distortion Info
#include "oculus.hpp"

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
	RM_OCULUS,
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

	/// @brief get the size of the Channel in homogeneous coordinates
	/// No setSize for now
	Rect<double> getSize(void) const
	{ return _size; }

	STEREO_EYE getStereoEyeCfg(void) const
	{ return _stereo_eye_cfg; }

	/// @todo this is really odd
	/// why does a Channel hold stereo config?
	/// for split screen stereo it's understandable because it's fixed
	/// for channels that handle both eyes it's really confusing.
	void setStereoEyeCfg(STEREO_EYE eye_cfg)
	{ _stereo_eye_cfg = eye_cfg; }
	
	/// Necessary to provide getter/setter because player does not exists
	/// when the Channel is created.
	Player *getPlayer(void) const
	{ return _player; }

	void setPlayer(Player *player)
	{ _player = player; }

	// Only meaningful for Oculus Rift
	vl::DistortionInfo const &getDistortionInfo(void)
	{ return _hmd_distortion_info; }
	void setDistortionInfo(vl::DistortionInfo const &info)
	{ _hmd_distortion_info = info; }

	/// Per frame statistics
	vl::scalar getLastFPS(void) const;

	/// @brief returns the number of triangles rendered in a frame
	/// For window targets this returns zero intentionally because
	/// windows can't provide statistics per viewport.
	size_t getTriangleCount(void) const;

	/// @brief returns the number of batches rendered in a frame
	/// For window targets this returns zero intentionally because
	/// windows can't provide statistics per viewport.
	size_t getBatchCount(void) const;

	void resetStatistics(void);

	StereoCamera &getCamera(void)
	{ return _camera; }

	Ogre::Viewport *getNative(void)
	{ return _viewport; }

	/// Methods
private :

	void _render_to_fbo(void);

	// passing stereo eye even though it's member variable because
	// the member variable looks iffy.
	void _oculus_post_processing(STEREO_EYE eye_cfg);

	/// nop if null parameter is passed here
	void _set_fbo_camera(vl::CameraPtr cam, std::string const &base_material);

	/// @param base_material is used to select post processing effects
	/// rtt has no post processing, oculus_rtt has distortion for Oculus Rift
	void _initialise_fbo(vl::CameraPtr camera, std::string const &base_material);

	/// nop if null parameter is passed here
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
		return 1 << (_viewport->getZOrder()+2); 
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
	Rect<double> _size;

	StereoCamera _camera;
	Ogre::Viewport *_viewport;

	RENDER_MODE _render_mode;
	uint32_t _fsaa;

	STEREO_EYE _stereo_eye_cfg;
	vl::Player *_player;

	vl::DistortionInfo _hmd_distortion_info;

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
