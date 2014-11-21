/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file channel.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#ifndef HYDRA_CHANNEL_HPP
#define HYDRA_CHANNEL_HPP


// Necessary for Window config and Wall
#include "base/config.hpp"

// need to be included before including any other stuff otherwise we get errors for GLEW
#include <HydraGL/src/OgreGLPrerequisites.h>

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
	RM_OCULUS,
};

inline std::string getRenderModeName(RENDER_MODE rm)
{
	switch(rm)
	{
	case RM_WINDOW:
		return "RM_WINDOW";
	case RM_FBO:
		return "RM_FBO";
	case RM_MRT_STEREO:
		return "RM_MRT_STEREO";
	case RM_DEFERRED:
		return "RM_DEFERRED";
	case RM_DEFERRED_MRT_STEREO:
		return "RM_DEFERRED_MRT_STEREO";
	case RM_OCULUS:
		return "RM_OCULUS";
	default:
		return "RM_UNKNOWN";
	}
	
}

/**	@brief Channel
 *	@todo remove direct Window rendering
 *	cleans up the the code a lot, both Deferred and Oculus use FBOs anyway
 *	FBO rendering is just slightly slower than direct window while
 *	it has the possibilities to use post processing effects etc.
 *
 *	@todo break into multiple versions, FBO and Deferred at least
 *
 *	@todo Deferred with Oculus?
 */
class Channel
{
public:
	/// Constructor
	/// @todo add FBO size as a parameter
	/// we can add it to channel config, no need to distribute it for now though
	/// @todo also move fsaa and rm to Channel config, they are annoying
	Channel(vl::config::Channel config, Ogre::Viewport *view, 
		RENDER_MODE rm, uint32_t fsaa, STEREO_EYE stereo_cfg, vl::Window *parent);

	void setCamera(vl::CameraPtr cam);

	std::string const &getName(void) const
	{ return _name; }

	void update(void);

	void draw(void);

	/// @brief get the size of the Channel in homogeneous coordinates
	/// No setSize for now
	Rect<double> getSize(void) const
	{ return _size; }

	/// @brief get the Texture size
	/// only valid for FBO render targets
	Rect<uint32_t> getTextureSize(void) const;

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

	Ogre::Viewport *getWindowViewport(void)
	{ return _win_viewport; }

	Ogre::Viewport *getRenderViewport(void);

	RENDER_MODE getRenderMode(void) const
	{ return _render_mode; }

	/// For Oculus
	void setCustomProjMatrix(bool use, vl::Matrix4 const proj);
	void setCustomViewMatrix(bool use, vl::Matrix4 const view);

	// Dirty hack to retrieve the texture ID for the RenderTarget
	// DO NOT use either of these on non-fbo targets or Deferred targets
	//
	// we are going to remove non FBO targets so these will just assert
	// fail for old Windowed rendering
	uint32_t getTextureID(void) const;

	uint32_t getFBOID(void) const;

	/// Methods
private :

	void _update_camera();

	void _render_to_fbo(void);

	/// nop if null parameter is passed here
	void _set_fbo_camera(vl::CameraPtr cam, std::string const &base_material);

	/// @param base_material is used to select post processing effects
	/// rtt has no post processing, oculus_rtt has distortion for Oculus Rift
	void _initialise_fbo(vl::CameraPtr camera, std::string const &base_material);

	/// nop if null parameter is passed here
	void _set_mrt_camera(vl::CameraPtr cam);

	void _initialise_mrt(vl::CameraPtr camera);

	Ogre::RenderTexture *_create_fbo(std::string const &name, Ogre::PixelFormat pf);

	void _create_screen_quad(std::string const &name, std::string const &material_name);

	void _deferred_geometry_pass(void);

	void _deferred_light_pass(void);

	uint32_t _get_window_mask(void)
	{
		// minimum 1<<2 because 1<<0 is the scene mask
		// and 1<<1 is the light mask
		return 1 << (_win_viewport->getZOrder()+2); 
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
	// saved variable for FBO creation, might not be actual texture size
	vl::vec2i _texture_size;

	StereoCamera _camera;
	/// Window viewport, for FBOs this is the non render viewport
	/// For windows this is the render viewport
	Ogre::Viewport *_win_viewport;
	/// For FBO targets this is the render viewport, for others it's null
	Ogre::Viewport *_fbo_viewport;

	RENDER_MODE _render_mode;
	uint32_t _fsaa;

	// -1 for left, +1 for right
	vl::scalar _stereo_eye;

	// Special Rendering surface
	// MRT is only available for Deferred shading
	// FBO is only available for FBO rendering
	Ogre::MultiRenderTarget *_mrt;
	Ogre::RenderTexture *_fbo;

	std::vector<Ogre::MaterialPtr> _fbo_materials;
	std::vector<Ogre::TexturePtr> _fbo_textures;

	Ogre::Camera *_rtt_camera;
	Ogre::SceneNode* _quad_node;

	int _draw_buffer;

	/// Oculus hacks
	bool _use_custom_view;
	bool _use_custom_proj;
	Matrix4 _custom_view;
	Matrix4 _custom_proj;

	vl::Window *_parent;

};	// class Channel

}	// namespace vl

#endif	// HYDRA_CHANNEL_HPP
